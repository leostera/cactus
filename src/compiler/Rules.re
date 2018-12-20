type template_target = {
  input: Fpath.t,
  output: Fpath.t,
  template: string,
};

type compile_target = {
  input: Fpath.t,
  output: Fpath.t,
};

type copy_target = {
  src: Fpath.t,
  dst: Fpath.t,
};

type compilation_unit = [
  | `Create_dir(Fpath.t)
  | `Compile(compile_target)
  | `Template(template_target)
  | `Copy(copy_target)
];

let cp = target =>
  Base.OS.(
    switch (target.src |> readfile |> writefile(target.dst)) {
    | exception _ => Logs.err(m => m("Something went wrong!"))
    | _ => ()
    }
  );

let template: (Fpath.t, template_target) => unit =
  (output_dir, target) => {
    let abs_out_path = Fpath.append(output_dir, target.output);
    let run_template = Str.(replace_first(regexp("{| document |}")));
    switch (
      target.input
      |> Base.OS.readfile
      |> (doc => run_template(doc, target.template))
      |> Base.OS.writefile(abs_out_path)
    ) {
    | exception _ => Logs.err(m => m("Something went wrong!"))
    | _ => ()
    };
  };

let compile: (Fpath.t, compile_target) => unit =
  (output_dir, target) => {
    let final_out_path = Fpath.append(output_dir, target.output);
    switch (
      target.input
      |> Base.OS.readfile
      |> Omd.of_string
      |> Omd.to_html(~pindent=true)
      |> Base.OS.writefile(final_out_path)
    ) {
    | exception _ => Logs.err(m => m("Something went wrong!"))
    | _ => ()
    };
  };

let mkdir = Base.OS.mkdirp;

/* TODO(@ostera): rewrite to use fprintf instead */
let pp = rule =>
  Format.(
    switch (rule) {
    | `Create_dir(name) =>
      print_string("mkdir " ++ name);
      print_newline();
    | `Compile({input, output, _})
    | `Template({input, output, _}) =>
      print_string(
        Fpath.to_string(input) ++ " => " ++ Fpath.to_string(output),
      );
      print_newline();
    }
  );
