open Rresult;

let read_rules = sexp => {
  open Sexplib0.Sexp;

  let rec find_template = values =>
    switch (values) {
    | [] => None
    | [List([Atom("template"), Atom(template_name)]), ..._xs] =>
      Some(template_name |> Fpath.v)
    | [_, ...xs] => find_template(xs)
    };

  let rec find_assets = values =>
    switch (values) {
    | [] => None
    | [List([Atom("assets"), Atom(".")]), ..._xs] => Some([Fpath.v(".")])
    | [List([Atom("assets"), ...assets]), ..._xs] =>
      let assets =
        assets
        |> List.fold_left(
             (acc, asset) =>
               switch (asset) {
               | Atom(filename) => [filename |> Fpath.v, ...acc]
               | _ => acc
               },
             [],
           );
      switch (assets) {
      | [] => None
      | xs => Some(xs)
      };
    | [_, ...xs] => find_assets(xs)
    };

  (sexp |> find_assets, sexp |> find_template);
};

let parse = (path, contents) =>
  contents
  |> Parsexp.Many.parse_string
  >>| read_rules
  >>| (
    ((assets, template)) =>
      Model.{
        path,
        dir: Fpath.parent(path),
        name: Fpath.basename(path),
        template,
        charset: "utf-8",
        assets,
      }
  )
  |> R.error_to_msg(
       ~pp_error=
         Parsexp.Parse_error.report(~filename=path |> Fpath.to_string),
     );
