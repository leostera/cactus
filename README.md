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
🌮 Done in 0.01s

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
