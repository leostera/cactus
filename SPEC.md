# Informal Specification

We define a simple model, consisting of a `cactus-project` and a `site`.

* **cactus-project**. Defines the root directory of a project.
* **site**. Defines a directory as a collection of compilable targets.

This allows us to easily define projects separately, and merge them together by
colocating them under a new `cactus-project` without needing to modify them at
all.

Internally, we use a few more concepts:

* `target`, a unit of work within the project file tree with a specific input,
  output, and an associated rule go generate the output from the input (such as
  "make a directory" or "compile markdown to html").

* `rule`, a description of how a `target` should be evaluated.

> Note: this nomenclature is heavily inspired by [Bazel](https://bazel.build)

### Execution

1. Look for `cactus-project` file in the current folder, to find the top of the
   directory tree. If no `cactus-project` is present, bail immediately.

2. Traverse that tree, looking for `site` files to identify directories that
   should be compiled.

3. For each `site`, find all the `targets` and build a dependency graph with it.

4. For each `target`, execute their associated `rule`.
