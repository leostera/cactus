let compile: (Model.project, Model.compilation_unit) => unit =
  (project, cunit) => {
    let final_out_path = Filename.concat(project.output_dir, cunit.output);
    Base.OS.readfile(cunit.input)
    |> Omd.of_string(~default_lang="en")
    |> Omd.to_html
    |> Base.OS.writefile(final_out_path);
  };
