# Derivation for quadratic programming problem

## Introduction to problem

HyLoRD's purpose is to take some bulk methylation profile and deconvolute this
by estimating the proportions of known/unknown cell types that make up the bulk 
profile. For the purpose of this page, we will assume that the reference matrix
(the amalgamation of methylation profiles for individual cell types) has been
formed already. Mathematically, we have the following relationship between the
reference matrix and the bulk profile:

\f{equation}{
    B_i = \sum^N_{k=1} p_kR_{i,k} 
    \label{eq:startingPoint}
\f}

Where:

- \f$B_i\f$ is the proportion of reads at CpG site \f$i\f$ that showed a
methylation signal (or hydroxymethylation signal)
- \f$p_k\f$ is the proportion of cell type \f$k\f$ (*e.g.* Neuron, Leukocyte)
- \f$R_{i,k}\f$ is a value from the reference matrix, corresponding with the
expected methylation signal for cell type \f$k\f$ and CpG site \f$i\f$.
- \f$N\f$ is the total number of cell types

Equation \f$\eqref{eq:startingPoint}\f$ is equivalent to saying that the bulk
profile is just a weighted average of the methylation profiles of each cell
type present in the data.

Considering \f$R_{i,k}\f$ and \f$B_i\f$ are both known, all that remains is to
find \f$p_k \: \forall k\in[N]\f$.

## Approach

To get good values for \f$p_k\f$, there are several approaches one could take.
For example, you could use some machine learning algorithm. For HyLoRD (and
many other cell type deconvolution algorithms), we elect to go for a more
direct approach by converting equation \f$\eqref{eq:startingPoint}\f$ into the
form of a 
[quadratic programming problem](https://en.wikipedia.org/wiki/Quadratic_programming).
The reason behind this choice is that many efficient solvers have been created
for quadratic programming problems (QPPs) over the years and it allows us to
be direct in estimating each \f$p_k\f$. This is opposed to a machine learning
approach of training some model, testing the model then applying it for each
possible dataset.

Specifically, this page will outline the proof that we can express 
\f$\eqref{eq:startingPoint}\f$ in the following form:

\f{equation}{
    minimize \: \frac{1}{2}\mathbf{x}^T\mathit{Q}\mathbf{x} + \mathbf{c}^T\mathbf{x},\\
    subject \; to \: \mathit{A}\mathbf{x} \leq \mathbf{b}
    \label{eq:qpp-form}
\f}

For equation \f$\eqref{eq:qpp-form}\f$, the matrices
\f$\mathit{Q},\mathit{A}\f$ and the vectors \f$\mathbf{b},\mathbf{c}\f$ are
known and \f$\mathbf{x}\f$ is the (solution) vector being solved for.

The goal of the following section is to convert equation
\f$\eqref{eq:startingPoint}\f$ into the form of equation
\f$\eqref{eq:qpp-form}\f$. In doing so, we can define a set of constraints that
makes sense for the problem at hand.

## Derivation

### Matrix form

First, we will rewrite equation \f$\eqref{eq:startingPoint}\f$ in matrix form:

\f{equation*}{
    \mathbf{B}^T = \sum_{k=1}^Np_k\mathbf{R_k}^T
\f}

Where:
- \f$\mathbf{B}\f$ is a \f$1 \times M\f$ column vector, where \f$M\f$ is the
number of CpG sites
- \f$\mathbf{R_k}\f$ is an \f$1 \times M\f$ column vector for each cell type k.

\f{equation}{
    \mathbf{B}^T = \mathbf{p}^T\mathit{R}^T
    \label{eq:matrix-form}
\f}

Here:
- \f$\mathbf{B}\f$ is a \f$1 \times M\f$ column vector, where \f$M\f$ is the
number of CpG sites
- \f$\mathbf{p}\f$ is a \f$1 \times N\f$ column vector, where \f$N\f$ is the
number of cell types
- \f$\mathit{R}\f$ is an \f$M \times N\f$ matrix

Below is proof that 
\f$\mathbf{p}^T\mathit{R}^T = \sum_{k=1}^Np_k\mathbf{R_k}^T\f$:

\f{aligned}
        \mathbf{p}^T\mathit{R}^T
            &= (p_1, \dots, p_N) \cdot
        \begin{pmatrix}
            r_{1,1} & r_{2,1} & \cdots & r_{n,1} \\
            r_{1,2} & r_{2,2} & \cdots & r_{n,2} \\
            \vdots & \vdots & \ddots & \vdots \\
            r_{1,N} & r_{2,N} & \cdots & r_{n,N}
        \end{pmatrix} \\
        &=(p_1r_{1,1} + \cdots + p_Nr_{1,N}, \dots, p_1r_{n,1} + \cdots + p_Nr_{n,N}) \\
        &=p_1(r_{1,1}, \dots, r_{n,1}) + (p_2r_{1,2} + \cdots + p_Nr_{1,N}, \dots, p_2r_{n,2} + \cdots + p_Nr_{n,N}) \\
        &\cdots \\
        &=p_1(r_{1,1}, \dots, r_{n,1}) + \cdots + p_N(r_{1,N}, \dots , r_{n,N}) \\
        &=p_1\mathbf{R}^T_1 + \cdots + p_N\mathbf{R}^T_N  \\
        &=\sum^N_{k=1}p_k\mathbf{R}^T_k
\f}

### Problem formulation

Currently, equation \f$\eqref{eq:matrix-form}\f$ is not in the form of a
quadratic programming problem. To create a QP from this equation, we need to
generate some objective function (a value to minimise). In our case, this is
actually very simple. We want to find some vector of cell proportions
(\f$p_k\f$), such that multiplying it with the reference matrix and retrieves a
vector as close as possible to the bulk profile.

This may look very arbitrary (that's maths for you). But we want to write this
as a least squares problem. The reason why is we need to write our problem as
a minimisation problem. \f$\eqref{eq:bad-start-objective-function}\f$
may seem like an obvious objective function to create. However, minimisation
could result in a large negative number instead of a value close to 0 which
isn't quite what we want. We could improve upon this by using the same
objective function, but taking the modulus (so this is no longer a problem),
but then we are stuck, unable to go any further in the derivation. The
'correct' choice to go with is actually to frame the problem as a least squares
problem like in \f$\eqref{eq:start-objective-function}\f$.

\f{equation}{
    minimize \: \mathbf{p}^T\mathit{R}^T - \mathbf{B}^T
    \label{eq:bad-start-objective-function}
\f}

\f{equation}{
    minimize \: ||\mathbf{p}^T\mathit{R}^T - \mathbf{B}^T||^2_2
    \label{eq:start-objective-function}
\f}

We can remove the transposes off of each component of
\f$\eqref{eq:start-objective-function}\f$ as it is equivalent. This yields:

\f{equation}{
    minimize \: ||\mathit{R}\mathbf{p} - \mathbf{B}||^2_2
    \label{eq:least-squares}
\f}

Note that we still need some constraints and apply a few matrix operations for
this to completely match up with the desired form given by
\f$\eqref{eq:qpp-form}\f$.

### Retrieving the correct objective function form

First we need to expand the L2 norm used in
\f$\eqref{eq:least-squares}\f$.


\f{aligned}{
    ||\mathit{R}\mathbf{p} - \mathbf{B}||^2_2
    &=(\mathit{R}\mathbf{p} - \mathbf{B})^T(\mathbf{p}\mathit{R} - \mathbf{B}) \\
    &=(\mathbf{p}^T\mathit{R}^T - \mathbf{B}^T)(\mathit{R}\mathbf{p} - \mathbf{B}) \\
    &=\mathbf{p}^T\mathit{R}^T\mathit{R}\mathbf{p} -\mathbf{B}^T\mathit{R}\mathbf{p} - \mathbf{p}^T\mathit{R}\mathbf{B} + \mathbf{B}^T\mathbf{B} \\
    &=\mathbf{p}^T\mathit{R}^T\mathit{R}\mathbf{p} -\mathbf{B}^T\mathit{R}\mathbf{p} - (\mathbf{p}^T\mathit{R}\mathbf{B})^T + \mathbf{B}^T\mathbf{B} \\
    &=\mathbf{p}^T\mathit{R}^T\mathit{R}\mathbf{p} -2\mathbf{B}^T\mathit{R}\mathbf{p} + \mathbf{B}^T\mathbf{B}
\f}


Next we can define a few variables:

- \f$\mathit{Q}:=\mathit{R}\mathit{R}^T\f$ 
- \f$\mathbf{c}^T:=-\mathbf{B}^T\mathit{R}\f$

Substituting these into our objective function \f$\eqref{eq:least-squares}\f$
we get:

\f{equation}{
    minimize \: \mathbf{p}^T\mathit{Q}\mathbf{p} + 2\mathbf{c}^T\mathbf{p} + \mathbf{B}^T\mathbf{B}
    \label{eq:substituted-Q-c}
\f}

This is close to the form required by \f$\eqref{eq:qpp-form}\f$, from here, we
need to multiply by 0.5 to get:

\f{equation*}{
    minimize \: \frac{1}{2}\mathbf{p}^T\mathit{Q}\mathbf{p} + \mathbf{c}^T\mathbf{p} + \frac{1}{2}\mathbf{B}^T\mathbf{B}
\f}

And then notice that our objective function has a constant in it (namely
\f$\frac{1}{2}\mathbf{B}^T\mathbf{B}\f$). Our choice of \f$\mathbf{p}\f$ has
no effect on this term of the objective function. Considering we don't actually
care what the value of the objective function is, just that it is minimised,
we can just remove this term entirely. This then retrieves the expected form
of the QPP (as seen in equation \f$\eqref{eq:objective-function}\f$).

\f{equation}{
    minimize \: \frac{1}{2}\mathbf{p}^T\mathit{Q}\mathbf{p} + \mathbf{c}^T\mathbf{p}
    \label{eq:objective-function}
\f}

### Constraints

We could apply trivial constraints by declaring \f$\mathit{A}\f$ as a zeros
matrix and be done with it. However, there are a few constraints that should
be applied to our QP. First, there is nothing constraining the minimum and 
maximum values for each cell proportion in the solution. In the context of our
deconvolution algorithm, this can lead to nonsensical solutions where some cell
type makes up a negative proportion of the bulk profile. Further, all of the
proportions should add up to 1.

\note
We could actually leave the second constraint (proportions add to 1) out and
just normalise the vector of proportions after. However, doing this opens up
the possibility of losing numerical stability (as the proportions could all
be very close to 0 or so big numerical overflow occurs).

HyLoRD uses [qpmad](https://github.com/asherikov/qpmad) to solve the QPP, and
this software allows for constraints on both sides (less than constraint and
greater than constraint). As a result of this, we can ensure that indeed the
proportions add up to 1 (bounded above and below by 1 is equivalent to
equal to 1) and that each proportion is bounded above by 1 and bounded below by
0.

### Putting it all together

After adding constraints to QPP \f$eqref{eq:final-objective-function}\f$ we
get:


\f{equation}{
    minimize \: \frac{1}{2}\mathbf{p}^T\mathit{Q}\mathbf{p} + \mathbf{c}^T\mathbf{p} \\
    subject \; to \: 1 \leq \mathit{A}\mathbf{p} \leq 1 \\
    \mathbf{1} \leq \mathbf{p} \leq \mathbf{1}
    \label{eq:retrieved-qpp}
\f}

Where:
- \f$\mathit{A}\f$ is a \f$N \times 1\f$ matrix of 1s

This might seem slightly different from the expected QPP form given in
\f$\eqref{eq:qpp-form}\f$, but we can combine the two constraints into the form
\f$\mathit{A}\mathbf{x} \leq \mathbf{b}\f$ if we wanted (in terms of the code
for HyLoRD however, this form is more accurate).

## Constrained least squares

The above is a reverse of the special case for \f$\eqref{eq:qpp-form}\f$ when
\f$\mathit{Q}\f$ is symmetric positive-definite. Such a QPP can be reduced
to a 'constrained least squares' problem (which is exactly what our original
problem is). Due to \f$\mathit{Q}\f$ being defined as
\f$\mathit{R}^T\mathit{R}\f$, we ensure that \f$\mathit{Q}\f$ in 
\f$\eqref{eq:retrieved-qpp}\f$ is indeed symmetric positive-definite. This is
important as the qpmad solver implements the 
[Goldfarb&Idnani algorithm](https://link.springer.com/chapter/10.1007/BFb0121074).

### On positive-definiteness

In order for our matrix \f$\mathit{Q}\f$ to actually be positive definite 
(definition only locks in positive semi-definite-ness). We need to be confident
that the columns of the original reference matrix are linearly independent.
Whilst we can't guarantee this, we can at least be sure that such a reference
matrix is incredibly unlikely to entered (unless intentional). The reason for
this is that our reference matrix will have thousands, hundreds of thousands of
rows (one for each CpG) of floating point numbers (methylation measured as
a proportion between 0 and 1). Due to how floating point numbers are stored
computationally (and the sheer number of rows), the chance of a column being
expressed as a linear combination of the other columns in the matrix is
basically zero.

In the event that the matrix is not positive-definite, you will see a message
alluding to Cholesky factorisation. Please check that you don't have any
duplicated columns in the reference matrix you provided. There is also the
extremely unlikely case where that the novel cell type generator managed to
produce a cell type that is a linear combination of the others in the matrix.
In such a case, running the tool again should fix this issue (unless the case
occurs again of course).

