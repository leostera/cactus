type compile_target = {
  input: Fpath.t,
  output: Fpath.t,
};

type compilation_unit = [ | `Create_dir(Fpath.t) | `Compile(compile_target)];

let compile: (Fpath.t, compile_target) => Lwt.t(unit) =
  (output_dir, target) => {
    let final_out_path = Fpath.append(output_dir, target.output);

    Lwt.catch(
      () =>
        Lwt.Infix.(
          target.input
          |> Fpath.to_string
          |> Base.OS.Async.readfile
          >|= (
            content =>
              content
              |> Omd.of_string
              |> Omd.to_html
              |> Markup.string
              |> Markup.parse_html
              |> Markup.signals
              |> Markup.pretty_print
              |> Markup.write_html
              |> Markup.to_string
          )
          >>= Base.OS.Async.writefile(final_out_path |> Fpath.to_string)
        ),
      exc =>
        Logs_lwt.err(m =>
          m("Something went wrong! Error: %s", exc |> Printexc.to_string)
        ),
    );
  };

let mkdir = path => path |> Fpath.to_string |> Base.OS.Async.mkdirp;

/* TODO(@ostera): rewrite to use fprintf instead */
let pp = rule =>
  switch (rule) {
  | `Create_dir(name) =>
    Printf.printf("mkdir %s\n%!", name |> Fpath.to_string)
  | `Compile({input, output}) =>
    Printf.printf(
      "%s\n%!",
      Fpath.to_string(input) ++ " => " ++ Fpath.to_string(output),
    )
  };
