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

let execute = (compiler, plan) => {
  open Lwt.Infix;
  let rec exec = (acc, p) =>
    switch (p) {
    | Leaf(target) =>
      Lwt.return([
        compiler(target)
        >>= (_ => Logs_lwt.debug(m => m("Compiled target: %s", target))),
        ...acc,
      ])
    | Node(target, ts) =>
      let (waiter, awakener) = Lwt.wait();
      Lwt.async(() =>
        compiler(target)
        >>= (_ => Logs_lwt.debug(m => m("Compiled Node target: %s", target)))
        >>= (
          _ =>
            ts
            |> List.map(exec(acc))
            |> Lwt_list.fold_left_s((a, b) => b >|= (c => c @ a), [])
            >>= (
              children =>
                Logs_lwt.debug(m => m("Compiled target: %s", target))
                >|= (_ => children)
            )
            >|= Lwt.wakeup_later(awakener)
        )
      );
      waiter;
    };
  Logs_lwt.debug(m => m("Executing build plan"))
  >>= (_ => exec([], plan) >>= Lwt.join);
};

let execute_p = (submit, compiler, plan) => {
  open Lwt.Infix;

  let (first_target, children) =
    switch (plan) {
    | Leaf(target) => (target, [])
    | Node(target, children) => (target, children)
    };

  let submit_one = t => {
    Logs.debug(m => m("Submitting %d tasks to worker...", t |> size));
    submit(t)
    >>= (
      x =>
        switch (x) {
        | None => Logs_lwt.err(m => m("Error submitting task to worker."))
        | Some(_) => Logs_lwt.debug(m => m("Tasks submitted successfully"))
        }
    );
  };

  compiler(first_target)
  >>= (() => children |> List.map(submit_one) |> Lwt.join);
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
