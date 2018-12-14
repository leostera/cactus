open Model;

let find_sites = root => {
  let rec crawl = (sites, top) =>
    if (top |> Sys.is_directory) {
      let files =
        switch (top |> Sys.readdir |> Array.to_list) {
        | [] => []
        | files =>
          List.concat([
            /* check if any of the current files are site files */
            files
            |> List.filter(name => name == Model.site_filename)
            |> List.map(Filename.concat(top))
            |> List.filter(path => Sys.is_directory(path) == false)
            |> List.map(path =>
                 Model.{
                   path,
                   dir: Filename.dirname(path),
                   name: Filename.basename(path),
                 }
               ),
            /* keep crawling the file tree */
            files
            |> List.map(Filename.concat(top))
            |> List.map(crawl([]))
            |> List.concat,
          ])
        | exception _ => []
        };
      files @ sites;
    } else {
      sites;
    };
  crawl([], root);
};

let read_project = root =>
  if (Model.project_filename |> Filename.concat(root) |> Sys.file_exists) {
    /* TODO(@ostera): read cactus-project to get the lang version and name */
    let name = "words";
    let output_dir = Filename.concat(root, "_public");
    Ok(Model.{root, name, output_dir, sites: find_sites(root)});
  } else {
    Error(`No_project_file(root));
  };

let find_docs = site =>
  site.dir
  |> Sys.readdir
  |> Array.to_list
  |> List.filter(filename => {
       let fullpath = Filename.concat(site.dir, filename);
       let is_dir =
         switch (fullpath |> Sys.is_directory) {
         | res => res
         | exception _ => false
         };
       ! is_dir;
     })
  |> List.filter(filename => {
       let ext = filename |> Filename.extension;
       ext == ".md";
     })
  |> List.map(filename => {
       let output_name = Filename.remove_extension(filename) ++ ".html";
       {
         input: Filename.concat(site.dir, filename),
         output: Filename.concat(site.dir, output_name),
       };
     });

let plan_build: Model.project => build_plan =
  project => {
    let compile_docs =
      project.sites
      |> List.map(find_docs)
      |> List.concat
      |> List.map(cunit => `Compile(cunit));
    let create_dirs =
      project.sites |> List.map(site => `Create_dir(site.dir));
    create_dirs @ compile_docs;
  };

let execute_build = project =>
  List.iter(
    fun
    | `Create_dir(path) => {
        let dir = Filename.concat(project.output_dir, path);
        print_string("   -> Creating dir: " ++ dir);
        print_newline();
        Base.OS.mkdirp(dir) |> ignore;
      }
    | `Compile(cunit) => {
        print_string(
          "   -> Compiling: "
          ++ cunit.input
          ++ " => "
          ++ Filename.concat(project.output_dir, cunit.output),
        );
        print_newline();
        Compiler.compile(project, cunit);
      },
  );

/* TODO(@ostera): pass in root as a pasameter */
let build = () => {
  let began_at = Unix.gettimeofday();

  print_string({j|🌵 Compiling project... |j});
  print_newline();
  let project = "./" |> read_project;
  switch (project) {
  | Ok(project) => project |> plan_build |> execute_build(project)
  | Error(err) => err |> Errors.to_string |> print_string
  };

  let finished_at = Unix.gettimeofday();
  let delta = finished_at -. began_at;
  print_string({j|🌮 Done in |j} ++ Printf.sprintf("%.2f", delta) ++ "s");
  print_newline();
};
