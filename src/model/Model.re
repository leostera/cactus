type site = {
  name: string,
  path: string,
  dir: string,
};

type project = {
  root: string,
  name: string,
  sites: list(site),
  output_dir: string,
};

let project_filename = "cactus-project";

let site_filename = "site";

type compilation_unit = {
  input: string,
  output: string,
};

type build_step = [ | `Create_dir(string) | `Compile(compilation_unit)];

type build_plan = list(build_step);
