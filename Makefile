.PHONY: deps
deps:
	opam install --deps-only .

.PHONY: build
build:
	dune build

.PHONY: watch
watch:
	dune build --watch

.PHONY: docs
docs:
	dune build @doc

.PHONY: fmt
fmt:
	dune build @fmt --auto-promote

.PHONY : clean
clean :
	dune clean
