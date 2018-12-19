module Parallel = Parallel;
module Async = Async;

type plan('target) =
  | Node('target, list(plan('target)))
  | Leaf('target);

let size = plan => {
  let rec count = (i, plan) =>
    switch (plan) {
    | Leaf(_) => i + 1
    | Node(_, plans) =>
      plans |> List.map(count(0)) |> List.fold_left((+), 1)
    };
  count(0, plan);
};

let partition: (int, plan('a)) => ('a, list(list(plan('a)))) =
  (count, plan) => {
    let (root, children) =
      switch (plan) {
      | Leaf(target) => (target, [])
      | Node(target, children) => (target, children)
      };

    let rec paths: (list(plan('a)), plan('a)) => list(plan('a)) =
      (acc, tree) =>
        switch (tree) {
        | Leaf(_) as leaf => [leaf, ...acc]
        | Node(target, deps) =>
          deps
          |> List.map(paths(acc))
          |> List.concat
          |> List.map(leaf => Node(target, [leaf]))
        };

    let buckets =
      children
      |> List.map(paths([]))
      |> List.concat
      |> Base.L.buckets(count);

    (root, buckets);
  };

let rec execute = (compiler, plan) =>
  switch (plan) {
  | Leaf(target) => compiler(target)
  | Node(target, children) =>
    compiler(target);
    children |> List.iter(execute(compiler));
  };

let rec execute_async: ('a => Lwt.t(unit), plan('a)) => Lwt.t(unit) =
  (compiler, plan) => {
    open Lwt.Infix;
    let (first_target, par_deps) =
      switch (plan) {
      | Leaf(target) => (target, [])
      | Node(target, children) => (target, children)
      };

    compiler(first_target)
    >>= (_ => Lwt_list.map_p(execute_async(compiler), par_deps))
    >|= (_ => ());
  };

/*
 // orchestrating the pool of workers and submitting work
 // figuring out what work can be submitted where
 */
let execute_p = (~jobs, compile, spawn_pool, plan, size) => {
  open Lwt.Infix;
  let (root, buckets) = partition(jobs, plan);
  let worker_count = buckets |> List.length;

  let (submit, teardown) = spawn_pool(worker_count);

  compile(root);
  Logs.debug(m
    /* we're not submitting the root of the tree, thus the size is decreased
       by one */
    => m("Submitting %d targets to %d workers", size - 1, worker_count));

  buckets |> Lwt_list.map_p(submit) >>= teardown |> Lwt_main.run;
  Logs.debug(m => m("Finished parallel execution."));
};

/* TODO(@ostera): rewrite to use fprintf instead */
let pp = (printer, graph) => {
  let rec print = (i, g) =>
    Format.(
      switch (g) {
      | Node(target, children) =>
        open_hovbox(i);
        print_string("(");
        printer(target);
        List.iter(print(i + 1), children);
        print_string(")");
        close_box();
      | Leaf(target) => printer(target)
      }
    );
  print(1, graph);
};
