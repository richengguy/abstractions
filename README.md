# abstractions
Generating abstract images with policy gradients.

## Building

### Initial Setup

'abstractions' uses [Conan](https://conan.io/) as the main software package
manager.  It needs to be installed before doing anything else.  There are two
ways to do this.

First, if you already have [conda](https://docs.conda.io/en/latest/) installed
then just run

```shell
conda env create
conda activate abstractions
```

to create the environment.

The other approach is just to use Python virtualenvs directly with

```shell
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```
