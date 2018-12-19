open Rresult;

let parse = (path, contents) =>
  contents
  |> Parsexp.Many.parse_string
  >>| (
    values =>
      switch (values) {
      | [
          Sexplib0.Sexp.List([Atom("template"), Atom(template_name)]),
          ..._xs,
        ] =>
        Some(template_name |> Fpath.v)
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
     );
