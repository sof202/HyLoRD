# Hybrid Long Read Deconvolution (HyLoRD) 

## Description

### Abstract

Cell type deconvolution is a critical tool in epigenome-wide association
studies (EWAS), enabling the estimation of cellular heterogeneity in bulk
tissue samples. While existing methods primarily focus on microarray or
whole-genome bisulfite sequencing (WGBS) data, the emergence of long-read
sequencing technologies-such as Oxford Nanopore Technologies (ONT)-presents new
opportunities for improved deconvolution. ONT allows for direct detection of
5-methylcytosine (5mC) and 5-hydroxymethylcytosine (5hmC) without bisulfite
conversion, enhancing the accuracy of epigenetic signal quantification.

Here, we present HyLoRD (Hybrid Long Read Deconvolution), a novel hybrid
deconvolution algorithm designed for long-read methylation data. HyLoRD
leverages a quadratic programming framework to estimate cell type proportions,
accommodating both reference-based and reference-free scenarios—enabling
prediction of both known and novel cell types. This flexibility addresses the
current scarcity of publicly available cell-sorted ONT methylation datasets.

Unlike neural network-based approaches, HyLoRD employs an efficient quadratic
programming solver (Goldfarb-Idnani), ensuring computational scalability
without requiring large training datasets. Additionally, HyLoRD uniquely
supports multi-signature deconvolution, incorporating both 5mC and 5hmC
modifications, which may enhance resolution in cell types with distinct
hydroxymethylation patterns (*e.g.*, neuronal populations).

### Introduction

Epigenetic cell type deconvolution is widely used to dissect cellular
heterogeneity in bulk tissue samples, with applications in EWAS and biomarker
discovery. Traditional methods rely on microarray or short-read bisulfite
sequencing (WGBS) data, but long-read sequencing platforms (particularly
ONT) offer distinct advantages, including:

- Native epigenetic detection: Direct calling of 5mC and 5hmC via basecallers
(e.g., Guppy, Dorado, Remora), reducing experimental noise.
- Improved genomic coverage: Long reads span more CpG sites, potentially
improving deconvolution accuracy.

Despite these advances, no existing deconvolution tool is optimized for
long-read methylation data. HyLoRD fills this gap by combining reference-based
and reference-free approaches, enabling robust proportion estimation even for
cell types absent in the user provided reference methylation matrix.

### Methodological Approach

HyLoRD formulates deconvolution as a quadratic programming problem (QPP),
optimizing cell proportions under biological constraints (e.g., non-negativity,
sum-to-one). Key features include:

- Hybrid Deconvolution
  - Incorporates prior knowledge from reference methylomes while allowing
  inference of novel cell types.
  - Mitigates biases from incomplete reference datasets.

- Computational Efficiency
  - Utilizes the Goldfarb-Idnani solver

- Multi-Signature Support
  - Optional integration of 5hmC signals, improving resolution in cell types
  with differential hydroxymethylation

### Advantages Over Existing Methods

Accuracy: Leverages long-read methylation calls, which are less susceptible to
PCR bias and coverage dropouts.

Flexibility: Compatible with sparse reference datasets, critical given the
limited availability of cell-sorted ONT data.

Interpretability: Quadratic programming provides transparent, deterministic
solutions.

### Applications

HyLoRD is particularly suited for:

- Validating cell sorting efficiency in single-cell experiments.
- Deconvolving neuronal tissues, where 5hmC signals provide discriminative
power.
- Analyzing archival or low-input samples where long-read sequencing is
advantageous.

### Future Directions
As ONT basecallers (e.g., Dorado) improve in accuracy, HyLoRD’s performance
will further benefit from higher-fidelity 5mC/5hmC calls. We also anticipate
extending HyLoRD to other long-read epigenetic modifications (e.g., 6mA) as
detection methods mature.

## Dependencies

- [CLI11](https://github.com/CLIUtils/CLI11/) (v2.5.0): Used for command-line
parsing. Licensed under BSD-3-Clause.
