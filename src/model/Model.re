type site = {
  charset: string,
  dir: Fpath.t,
  name: string,
  path: Fpath.t,
};

type project = {
  root: Fpath.t,
  name: string,
  output_dir: Fpath.t,
};

let project_filename = "cactus-project" |> Fpath.v;

let site_filename = "site" |> Fpath.v;
