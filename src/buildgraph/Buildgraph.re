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

let execute_p = (~jobs, submit, compiler, plan) => {
  open Lwt.Infix;
  let (first_target, par_deps) =
    switch (plan) {
    | Leaf(target) => (target, [])
    | Node(target, children) => (target, children)
    };

  compiler(first_target);

  let submit' = x =>
    submit(x)
    >>= (
      submission =>
        switch (submission) {
        | None => Logs_lwt.err(m => m("Failed to submit task"))
        | Some () => Logs_lwt.debug(m => m("Task submitted!"))
        }
    );

  let buckets = par_deps |> Base.L.buckets(jobs);
  Logs.debug(m =>
    m(
      "Submitting %d children targets to %d workers in %d buckets",
      par_deps |> List.length,
      jobs,
      buckets |> List.length,
    )
  );
  buckets |> Lwt_list.map_p(submit');
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
