type compile_target = {
  input: Fpath.t,
  output: Fpath.t,
  template: option(string),
};

type compilation_unit = [ | `Create_dir(Fpath.t) | `Compile(compile_target)];

let compile: (Fpath.t, compile_target) => unit =
  (output_dir, target) => {
    let final_out_path = Fpath.append(output_dir, target.output);
    switch (
      target.input
      |> Base.OS.readfile
      |> Omd.of_string
      |> Omd.to_html(~pindent=true)
      |> (
        doc =>
          switch (target.template) {
          | Some(t) => Str.(replace_first(regexp("{| document |}"), doc, t))
          | _ => doc
          }
      )
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
    | `Compile({input, output, _}) =>
      print_string(
        Fpath.to_string(input) ++ " => " ++ Fpath.to_string(output),
      );
      print_newline();
    }
  );
