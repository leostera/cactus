type site = {
  name: string,
  path: Fpath.t,
  dir: Fpath.t,
  charset: string,
};

type project = {
  root: Fpath.t,
  name: string,
  output_dir: Fpath.t,
};

let project_filename = "cactus-project" |> Fpath.v;

let site_filename = "site" |> Fpath.v;
