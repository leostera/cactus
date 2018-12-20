type site = {
  charset: string,
  template: option(Fpath.t),
  assets: option(list(Fpath.t)),
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
