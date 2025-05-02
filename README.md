<p align="center">
  <img style="border-radius:10px" src="https://github.com/user-attachments/assets/b2d6eb3b-e34f-4961-9199-3b5070b567d3" />
</p>


</p>
<p align="center">
    <a href="https://github.com/sof202/HyLoRD/actions/workflows/docs.yml" alt="Build status">
      <img src="https://img.shields.io/github/actions/workflow/status/sof202/HyLoRD/docs.yml?style=for-the-badge&color=orange" /></a>
    <a href="https://github.com/sof202/HyLoRD/commits/main/" alt="Commit activity">
        <img src="https://img.shields.io/github/commit-activity/m/sof202/HyLoRD?style=for-the-badge&color=orange" /></a>
    <a href="https://github.com/sof202/HyLoRD/blob/main/LICENSE" alt="License">
        <img src="https://img.shields.io/github/license/sof202/HyLoRD?style=for-the-badge&color=orange" /></a>

</p>

## Table of contents

* [Description](#description)
  * [Introduction](#introduction)
  * [Methodological Approach](#methodological-approach)
  * [Advantages Over Existing Methods](#advantages-over-existing-methods)
  * [Applications](#applications)
* [Installation](#installation)
  * [Prebuilt binary](#prebuilt-binary)
  * [Install from source](#install-from-source)
* [Running](#running)
  * [Further details](#further-details)
* [Documentation](#documentation)

## Description

HyLoRD (Hybrid Long Read Deconvolution) is a cell type deconvolution
tool. It utilitises a hybrid (rather than reference-based/free) approach
to deconvoluting ONT long read sequencing data.

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

HyLoRD 
[formulates deconvolution as a quadratic programming problem (QPP)](https://sof202.github.io/HyLoRD/md__hy_lo_r_d_2docs_2high-level-docs_2qpp-derivation.html)
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
  with differential hydroxymethylation (particularly useful when deconvoluting
  bulk data from brain tissue for example)

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

## Installation

### Prebuilt binary

Download the latest release from the 
[releases page](https://github.com/sof202/HyLoRD/releases/) and run:

```sh
# Extract binary
tar -xzvf hylord-[version]-[distribution].tar.gz
```

### Install from source

The build process for HyLoRD is carried out via CMake, however the process is
made easier via a Makefile wrapper. To install:

```sh
make CMAKE_BUILD_TYPE=Release
sudo make install
```

To install to the non-default location

```sh
# Note that the path provided MUST NOT be a relative path, but a full path
make CMAKE_BUILD_TYPE=Release CMAKE_INSTALL_PREFIX=path/to/hylord
sudo make install
```

After completing this process make sure that `path/to/hylord` is on your
`$PATH` with `which hylord`. If not found, you will need to update your `PATH`
environment variable (details of which can be found
[here](https://www.digitalocean.com/community/tutorials/how-to-view-and-update-the-linux-path-environment-variable)).

You can verify that the installation completed by running:

```sh
hylord --version
```

See `BUILD.md` for further details, including dependencies.

## Running

To get started with HyLoRD, you will need the 
[required input files](https://sof202.github.io/HyLoRD/md__2home_2sof202_2_tools__and___repositories_2_hy_lo_r_d_2docs_2high-level-docs_2inputs-outputs.html).

After this you can run HyLoRD with:

```bash
hylord -r path/to/reference_matrix.bed path/to/bedmethyl.bed
```

### Further details

Generally when using HyLoRD, the command would look something like:

```bash
hylord \
  -r path/to/reference_matrix.bed \
  -l path/to/cell_list.txt \
  -c path/to/cpg_list.bed \
  --additional-cell-types 3 \
  path/to/bedmethyl.bed
```

Further command line options are explained by running:

```bash
hylord -h
```

## Documentation

Full documentation for HyLoRD can be found
[here](https://sof202.github.io/HyLoRD). You can also head over to `BUILD.md`
for details on how to build these pages locally with `doxygen`.
