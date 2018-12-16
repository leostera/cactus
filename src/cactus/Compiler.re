let compile: (Model.project, Model.compilation_unit) => Lwt.t(unit) =
  (project, cunit) => {
    let final_out_path = Filename.concat(project.output_dir, cunit.output);
    Base.OS.(
      Lwt.(
        readfile(cunit.input)
        |> map(Omd.of_string)
        |> map(Omd.to_html)
        >>= writefile(final_out_path)
      )
    );
  };
