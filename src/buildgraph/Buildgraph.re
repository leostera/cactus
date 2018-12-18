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

let execute_p = (submit, compiler, plan) => {
  let rec exec = (acc, p) =>
    switch (p) {
    | Leaf(target) => [target, ...acc]
    | Node(target, children) =>
      compiler(target);
      children |> List.map(exec(acc)) |> List.concat;
    };
  plan
  |> exec([])
  |> Base.L.bucket(~bucket_size=4)
  |> List.map(t => submit(t) |> Lwt.map(_ => ()))
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
