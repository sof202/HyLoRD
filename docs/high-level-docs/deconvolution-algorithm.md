# Deconvoluiton algorithm {#algorithm}

## High level explanation of main pipeline

There are two different algorithms that can be used by HyLoRD. One is the
reference-based approach, where all cell types are accounted for in reference
matrix and `--additional-cell-types` is set to zero. The other is the
hybrid/reference-free approach where the reference matrix is not
completed/given respectively. Considering the current lack of cell sorted ONT
long read sequencing data, the second approach is more likely to be useful.

### Reference-based approach {#reference-based}

In this scenario, the deconvolution algorithm consists of a single step where
the following question is converted into a quadratic programming problem.

> What cell type proportions when combined with the reference matrix best
> describe the bulk methylation profile (present in the bedmethyl file from ONT
> data)?

For information on how this problem is converted to a quadratic programming
problem, visit [this page](@ref derivation).

After converting the input data into the desired form, HyLoRD offloads the
solving of said quadratic programming problem to a library called
[qpmad](https://github.com/asherikov/qpmad). This process will then find the
optimal cell type proportions that describe the input bulk bedmethyl data.

### Hybrid/reference-free approach

This approach is very similar to the 
[reference-based approach](@ref reference-based), except that there is now
a couple of extra steps.

First, the reference matrix is either incomplete or missing, so we need to
[generate methylation (and hydroxymethylation) profiles](@ref novel-cell-profiles)
for novel cell types. Once this step is completed, HyLoRD completes the
following two steps in a loop:

1) Converts the data into the form of a 
[quadratic programming problem](@ref reference-based) and solves the QPP using
qpmad

2) [Updates the reference matrix](@ref reference-matrix-updating) using the
acquired cell proportions from step 1.

These two steps repeat until at least one of the following criteria are met:

1) The maximum number of iterations is exceeded (can be user specified with
`--max-iterations`)

2) The change in cell type proportions between iterations is below the
convergence threshold (can be user specified with `--convergence-threshold`).

#### How novel cell methylation profiles are created {#novel-cell-profiles}

This could be completed with a trivial approach like setting each CpG's
methylation status to a random value (for each additional cell type required),
but HyLoRD takes a sligthly more sophisticated approach. Using previously
obtained bedmethyl files from [modkit](https://github.com/nanoporetech/modkit)
an approximate cumulative distribution function (CDF) was created (these CDFs
used can be found in `random.hpp`). The decision to use an approximate CDF was
because we just want a methylation profile that is plausible, not a completely
accurate depiction of some cell type (as that is completely out of the
question).

The CDFs for each epigenomic signal were generated in the following two steps:

##### Step 1 - Extract methylation

Some bedmethyl files (obtained from modkit) were combined, raising
the average read depth. Then the fraction modified (field 11) was extracted
where the modified base code was 'm' (methylated) and the valid coverage
(score) was greater than 10. This was done with the following `awk` command:

```sh
awk '$4=="m" && $5>=10 {print $11}' .../bedmethyl.bed > methylations.txt
```

This process was repeated for the hydroxymethylation signal as well (fourth
field set to 'h'). The minimum read depth of 10 (although arbitrary) was chosen
with two considerations:

1) This is count based data, a low read depth gives very little statistical
power. For example, a read depth of 1 only allows the fraction modified to be 0
or 1 (which can't really be trusted as a ground truth).

2) Picking a read depth that is too high would result in too many CpG sites
being thrown out, which would result in an inaccurate representation of
methylation proportions across the epigenome.

##### Step 2 - Obtain CDF

To obtain the CDF, the following python code was utiltised:

```python
import numpy as np
import pandas as pd

methylations = pd.read_csv("methylations.txt")

# values are discretised to nearest 10 to get a smaller in size cdf
methylations = np.round(methylations, -1) 
values, counts = np.unique(methylations, return_counts=True)
probabilities = counts / counts.sum()

# cdf maps to 0,0.1,0.2,...,1.0
cdf = np.cumsum(probabilities)
```

#### How the reference matrix is updated {#reference-matrix-updating}

The original reference matrix that is generated is highly likely to be poor
quality. The methylation proportions present for the novel cell types make
sense on a macro level, but not for individual CpG sites. For example, some
CpG sites are methylated in the vast majority of cell types (or none at all),
however the generated reference matrix will likely not reflect this quality.

Upon obtaining estimates for the proportions of each cell type, the reference
matrix can be updated. To update the reference matrix, we turn to the
[relationship](@ref matrix-form) between the bulk methylation profile, the
reference matrix and the cellular proportions estimated from the quadratic
programming problem:

\f{equation*}{
    \mathbf{B} \simeq \mathit{R}\mathit{p}
\f}

Where: 

- \f$\mathbf{B}\f$ is the bulk methylation profile
- \f$\mathbf{p}\f$ is the estimated cell proportions
- \f$\mathit{R}\f$ is the reference matrix

To update our reference matrix, we need to apply some matrix manipulation.
First, we need to split our reference matrix and cell proportions in two. Some
of the reference matrix is set in stone (methylation profiles provided by user)
and we don't want to overwrite these.

\f{equation}{
    \mathbf{B} \simeq \mathit{R}_k\mathbf{p}_k + \mathit{R}_l\mathbf{p}_l
    \label{eq:split-formula}
\f}

Equation \f$\eqref{eq:split-formula}\f$ showcases this splitting process. The
\f$k\f$ left-most columns  in the reference matrix (and the first \f$k\f$ cell
proportions) are the \f$k\f$ cell type profiles that are given by the user
(where \f$k\geq 0\f$). The \f$l\f$ remaining columns of the reference matrix
(and last \f$l\f$ cell proportions) are what's left to change.

We can now update the \f$l\f$ right most columns of the reference matrix by
rearranging equation \f$\eqref{eq:split-formula}\f$ and multiplying by the
[psuedo-inverse](https://en.wikipedia.org/wiki/Moore–Penrose_inverse) of
\f$\mathbf{p}_l\f$. The result of these operations is given in equation
\f$\eqref{eq:new-reference-matrix-cols}\f$:

\f{equation}{
    \mathit{R}_l \simeq (\mathbf{B} - \mathit{R}_k\mathbf{p}_k)
        \bigg(\frac{\mathbf{p}_l^T}{\mathbf{p}_l^T\mathbf{p}}_l\bigg)
    \label{eq:new-reference-matrix-cols}
\f}

From here, the reference matrix can be updated by replacing the right \f$l\f$
columns with the result of the formula given in
\f$\eqref{eq:update-reference-matrix}\f$.


\f{equation}{
    (\mathbf{B} - \mathit{R}_k\mathbf{p}_k)
        \bigg(\frac{\mathbf{p}_l^T}{\mathbf{p}_l^T\mathbf{p}}_l\bigg)
    \label{eq:update-reference-matrix}
\f}
