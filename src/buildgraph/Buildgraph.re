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
    >>= (_ => Logs_lwt.debug(m => m("Async compiled target")))
    >|= (_ => List.map(execute_async(compiler), par_deps))
    >>= Lwt.join
    >>= (_ => Logs_lwt.debug(m => m("Async compiled deps")));
  };

let execute_p = (submit, compiler, plan) => {
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
        | Some(task) =>
          Logs_lwt.debug(m => m("Submitted task"))
          >>= (_ => task)
          >>= (_ => Logs_lwt.debug(m => m("Task completed")))
        }
    );

  Logs.debug(m => m("Submitting %d targets", par_deps |> List.length));
  par_deps |> List.map(submit') |> Lwt.join;
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
