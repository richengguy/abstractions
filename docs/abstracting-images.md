# Generating Abstractions

Generating an image abstraction is done via the `abstractions` app.  The app has
two modes:

* Generation
* Rendering

The "generation" mode attempts to find an abstraction and then stores the
results into a JSON file.  "Rendering" uses the JSON file to recreate the
abstraction.

## Creating a new abstraction

The `generate` command is used to generate, or find, an abstract representation
of an image:

```shell
abstractions generate some-image.jpg output
```

This will create `output.png` and `output.json` files with the abstraction
results.  The PNG file is a preview image showing what the abstraction looks
like while the JSON file is the actual abstraction.

:::{table} Image [2018](https://github.com/BIDS/BSDS500/blob/master/BSDS500/data/images/test/2018.jpg) from the [Berkley Segmentation Data Set 500](https://github.com/BIDS/BSDS500).

| Original | Abstraction |
|----------|-------------|
|![](examples/original/2018.jpg)|![](examples/2018.png)|

:::

There are a few different options available while running the optimization
algorithm; use `abstractions generate --help` to see all of them.  Some of the
more important ones are:

| Option | Description |
|--------|-------------|
| `-n <num>` | The number of iterations to run.  This is set to a very high number (10'000) by default but it can often be set much lower as the optimizer will converge to a solution long before then. |
| `-k <num>` | The number of shapes to use.  The more shapes, the more complicating the abstraction will be. |
| `-t <shape>` | Select the type of shape to use.  This can be repeated to use multiple shapes. Must be one of `circles`, `rectangles`, or `triangles`.|
| `-a <alpha-scale>` | Scales shapes alpha values by this amount.  The default is `1.0`, meaning shapes are allowed to be completely opaque.  Setting this to something lower will cause shapes to never be fully opaque. |

### Visualizing the optimization process

The optimization used to generate the abstraction can be visualized with the
`--save-intermediate` option.  This will cause `abstractions` to render an image
every 25 iterations.  This can be converted into a GIF, for example, using
[ImageMagick](https://imagemagick.org/index.php):

```shell
abstractions generate --save-intermediate steps some-image.jpg output
magick -delay 4 -loop 0 steps/*.png animation.gif
```

Or an .mp4 with [FFmpeg](https://ffmpeg.org/):

```shell
ffmpeg -framerate 24 -pattern_type glob -i 'steps/*.png' -c:v libx264 -pix_fmt yuv420p -qp 20 animation.mp4
```

:::{video} examples/2018.mp4
:autoplay:
:loop:
:align: center
:::

## Rendering abstractions

:::{important}
abstractions uses [Blend2D](https://blend2d.com/) both for rendering and image
handling.  The library doesn't currently support writing JPEG files so
abstractions will only ever output PNG files.  It will produce an error when
attempting to write to a JPEG file.
:::

A abstraction can be rendered with

```shell
abstractions render --width 500 output.json abstraction.png
```

This will render the abstraction in `output.json` to `abstraction.png`, setting
the width to 500 pixels.  The aspect ratio is stored inside of the JSON file so
the output size can be any size.  Switch `--width` to `--height` to set output
height instead.

Creating an abstraction that's the same size as the input is done with

```shell
WIDTH= # get the width of the input image
abstractions generate some-image.jpg output
abstractions render --width $WIDTH output.json abstractions.png
```
