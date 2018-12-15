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

let read_project = (root, output_dir) =>
  if (Model.project_filename |> Filename.concat(root) |> Sys.file_exists) {
    /* TODO(@ostera): read cactus-project to get the lang version and name */
    let name = "words";
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

let execute_build = (project, cunits) => {
  let began_at = Unix.gettimeofday();

  Logs.debug(m =>
    m("About to process %d compilation units...", cunits |> List.length)
  );
  List.iter(
    fun
    | `Create_dir(path) => {
        let dir = Filename.concat(project.output_dir, path);
        Logs.info(m => m("Creating dir: %s", dir));
        Base.OS.mkdirp(dir) |> ignore;
      }
    | `Compile(cunit) => {
        Logs.info(m =>
          m(
            "Compiling: %s => %s",
            cunit.input,
            Filename.concat(project.output_dir, cunit.output),
          )
        );
        Compiler.compile(project, cunit);
      },
    cunits,
  );
  Logs.debug(m => {
    let finished_at = Unix.gettimeofday();
    let delta = finished_at -. began_at;
    m(
      {j|Processes %d compilation units in %.3f s|j},
      cunits |> List.length,
      delta,
    );
  });
};

/* TODO(@ostera): pass in root as a pasameter */
let build = (_flags, project_root, output_dir) => {
  let began_at = Unix.gettimeofday();

  Logs.app(m => m({j|🌵 Compiling project... |j}));

  let project = read_project(project_root, output_dir);
  switch (project) {
  | Ok(project) => project |> plan_build |> execute_build(project)
  | Error(err) => Logs.err(m => err |> Errors.to_string |> m("ERROR: %s"))
  };

  Logs.app(m => {
    let finished_at = Unix.gettimeofday();
    let delta = finished_at -. began_at;
    let has_errors = Logs.err_count() > 0;
    let msg = if (has_errors) {{j|💀 Failed in|j}} else {{j|🌮 Done in|j}};
    m("%s %0.3fs", msg, delta);
  });
};
