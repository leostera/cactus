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

let read_site = path => path |> Base.OS.readfile |> Site.parse(path);
