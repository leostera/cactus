module Errors = Errors;

let read_project = (root, output_dir) => {
  let root = root |> Fpath.to_dir_path |> Fpath.normalize;
  let output_dir = output_dir |> Fpath.to_dir_path |> Fpath.normalize;

  let res =
    Model.project_filename
    |> Fpath.append(root)
    |> Bos.OS.File.exists
    |> Rresult.R.ignore_error(~use=_ => false);

  if (res) {
    /* TODO(@ostera): read cactus-project to get the lang version and name */
    let name = "words";
    Ok(Model.{root, name, output_dir});
  } else {
    Error(`No_project_file(root));
  };
};

/* TODO(@ostera): read path file, validate rules, and turn them into a
 * Model.site record */
let read_site = path =>
  Rresult.(
    path
    |> Base.OS.readfile
    |> Parsexp.Many.parse_string
    >>| (
      values =>
        switch (values) {
        | [
            Sexplib0.Sexp.List([Atom("template"), Atom(template_name)]),
            ..._xs,
          ] =>
          Logs.debug(m => m("Template file name: %s", template_name));
          Some(template_name |> Fpath.v);
        | _ => None
        }
    )
    >>| (
      template =>
        Model.{
          path,
          dir: Fpath.parent(path),
          name: Fpath.basename(path),
          template,
          charset: "utf-8",
        }
    )
    |> R.error_to_msg(
         ~pp_error=
           Parsexp.Parse_error.report(~filename=path |> Fpath.to_string),
       )
  );
