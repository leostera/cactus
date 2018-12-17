type compile_target = {
  input: string,
  output: string,
};

type compilation_unit = [ | `Create_dir(string) | `Compile(compile_target)];

/* TODO(@ostera): remove project dependency */
let compile = (output_dir, cunit) => {
  let final_out_path = Filename.concat(output_dir, cunit.output);
  Base.OS.readfile(cunit.input)
  |> Omd.of_string
  |> Omd.to_html
  |> Markup.string
  |> Markup.parse_html
  |> Markup.signals
  |> Markup.pretty_print
  |> Markup.write_html
  |> Markup.to_string
  |> Base.OS.writefile(final_out_path);
};

let mkdir = Base.OS.mkdirp;

/* TODO(@ostera): rewrite to use fprintf instead */
let pp = rule =>
  Format.(
    switch (rule) {
    | `Create_dir(name) =>
      print_string("mkdir " ++ name);
      print_newline();
    | `Compile({input, output}) =>
      print_string(input ++ " => " ++ output);
      print_newline();
    }
  );
