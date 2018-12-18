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
    | Leaf(target) => Lwt.return([compiler(target), ...acc])
    | Node(target, ts) =>
      let (waiter, awakener) = Lwt.wait();
      /*
         list(Lwt.t(list(Lwt.t(unit))))
         Lwt.t(list(Lwt.t(unit)))
       */
      Lwt.async(() =>
        compiler(target)
        >>= (
          _ =>
            ts
            |> List.map(exec(acc))
            |> Lwt_list.fold_left_s((a, b) => b >|= (c => c @ a), [])
            >|= Lwt.wakeup_later(awakener)
        )
      );
      waiter;
    };
  exec([], plan) >>= Lwt.join;
};

let execute_p = (~jobs, submit, compiler, plan) => {
  let rec exec = (acc, p) =>
    switch (p) {
    | Leaf(target) => [target, ...acc]
    | Node(target, children) =>
      compiler(target);
      children |> List.map(exec(acc)) |> List.concat;
    };
  plan
  |> exec([])
  |> Base.L.buckets(jobs)
  |> List.map(t =>
       Lwt.(
         Logs_lwt.debug(m =>
           m("Submitting %d tasks to worker...", t |> List.length)
         )
         >>= (_ => submit(t))
         |> Lwt.map(_ => ())
       )
     )
  |> Lwt.join;
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
