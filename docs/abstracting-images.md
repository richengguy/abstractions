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

:::{attention}
Include an example!!!
:::

There are a few different options available while running the optimization
algorithm; use `abstractions generate --help` to see all of them.  Some of the
more important ones are:

| Option | Description |
|--------|-------------|
| `-n <num>` | The number of iterations to run.  This is set to a very high number (10'000) by default but it can often be set much lower as the optimizer will converge to a solution long before then. |
| `-k <num>` | The number of shapes to use.  The more shapes, the more complicating the abstraction will be. |
| `-t <shape>` | Select the type of shape to use.  This can be repeated to use multiple shapes. |
| `--save-intermediate <dir>` | Save intermediate results to `<dir>`. |

### Visualizing the optimization process

The optimization used to generate the abstraction can be visualized with the
`--save-intermediate` option.  This will cause `abstractions` to render an image
every 25 iterations.  This can be converted into a GIF, for example, using
[ImageMagick](https://imagemagick.org/index.php):

```shell
abstractions generate --save-intermediate steps some-image.jpg output
magick -delay 4 -loop 0 steps/*.png animation.gif
```

:::{attention}
Include an example!!!
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

Because the generation stage may use a smaller image, creating an abstraction
the same size as the input is done with

```shell
WIDTH= # get the width of the input image
abstractions generate some-image.jpg output
abstractions render --width $WIDTH output.json abstractions.png
```
