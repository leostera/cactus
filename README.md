# 🌵 Cactus — A composable static site generator

Cactus is a reaction to the amount of static site generators out there
that enforce their structures on you. Cactus does very little. If you open it
up, you'll find it's full of water.

### Installing

`cactus` requires a working OCaml toolchain with opam. If you have it, you can
just pin the repository:

```sh
opam pin --dev omd
opam pin add cactus https://github.com/ostera/cactus.git
```

If you'd rather run from source you can also `git clone` and `make install`:

```sh
opam pin --dev omd
git clone https://github.com/ostera/cactus path/to/projects
cd path/to/projects
make install
```

Worht noting that `make install` just calls `dune install`.


## Getting Started

Cactus works in a very simple way. In fact it's almost silly how simple it is.
If you put a `cactus-project` file on the root of your project, cactus will look
throughout your whole project for `site` files.

`site` files simply tell `cactus` that this particular folder should be compiled
into a website.

So if you have your posts in the following structure:

```sh
my/website λ tree
.
├── pages
│   ├── First-post.md
│   └── Some-other-post.md
└── sections
    ├── about.md
    ├── hire-me.md
    └── projects.md
```

You just need to `touch` a few files:

```sh
my/website λ touch cactus-project
my/website λ touch pages/site sections/site
```

And you can run `cactus` to compile the website using the same tree structure
under a `_public` folder:

```sh
my/website λ cactus build
🌵 Compiling project...
🌮 Done in 0.002s

my/website  λ tree
.
├── _public
│   ├── pages
│   │   ├── First-post.html
│   │   └── Some-other-post.html
│   └── sections
│       ├── about.html
│       ├── hire-me.html
│       └── projects.html
├── cactus-project
├── pages
│   ├── First-post.md
│   ├── Some-other-post.md
│   └── site
└── sections
    ├── about.md
    ├── hire-me.md
    ├── projects.md
    └── site
```

Which you can readily serve however you feel like. Upload to S3, Now, GCS,
Github pages, or pretty much wherever.

When in doubt, check out the `example` folder. All of the features will be
showcased there.

### Templating

You'll quickly notice that the bare compilation from Markdown to HTML doesn't
quite fit all use-cases. To alleviate this `cactus` lets you specify in your
`site` file a template file to be used for all the Markdown files within that
specific site.

Say you wanted to wrap all of the pages from the example above in a common
markup: add a `<meta charset="utf-8">` to all of them. You'd write a template
file:

```html
<html>
  <head>
    <meta charset="utf-8">
  </head>
  <body>
    {| document |}
  </body>
</html>
```

And in your `site` file you'd point to it:

```lisp
(template "path/to/template.html")
```

Voila! That's all it takes to get the templating up and running. It's very basic
at the moment, but it'll get you quite far! The next step is to provide better
support for building pages with arbitrary logic, possibly by letting you specify
a module to be used for processing each file.

### Assets

To copy assets (any supporting file to your site) you can use the `(assets
...)` rule:

```lisp
(assets
  style.css
  logo.svg
  bg_music.midi)
```

And they will be automatically copied from their location, relative to the
`site` file.
