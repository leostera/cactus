open Model;

let find_sites: string => Lwt.t(list(site)) =
  root =>
    root
    |> Base.OS.readdir
    |> Lwt_stream.filter(name =>
         Filename.basename(name) == Model.site_filename
       )
    |> Lwt_stream.map(path =>
         Model.{
           path,
           dir: Filename.dirname(path),
           name: Filename.basename(path),
         }
       )
    |> Lwt_stream.to_list;

let read_project = (root, output_dir) =>
  Lwt.(
    Model.project_filename
    |> Filename.concat(root)
    |> Base.OS.file_exists
    >>= (
      exists =>
        if (exists) {
          find_sites(root)
          |> map(sites => {
               /* TODO(@ostera): read cactus-project to get the lang version and name */
               let name = "words";
               Ok(Model.{root, name, output_dir, sites});
             });
        } else {
          `No_project_file(root) |> return_error;
        }
    )
  );

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

let plan_build: Model.project => Lwt.t(build_plan) =
  project => {
    let compile_docs =
      project.sites
      |> List.map(find_docs)
      |> List.concat
      |> List.map(cunit => `Compile(cunit));
    let create_dirs =
      project.sites |> List.map(site => `Create_dir(site.dir));
    let targets = create_dirs @ compile_docs;
    Logs_lwt.debug(m =>
      m(
        "Build planned: %d targets ( %d compilations, %d directories )",
        targets |> List.length,
        compile_docs |> List.length,
        create_dirs |> List.length,
      )
    )
    |> Lwt.map(() => targets);
  };

let execute_rule = (project, rule) =>
  Lwt.(
    switch (rule) {
    | `Create_dir(path) =>
      let dir = Filename.concat(project.output_dir, path);
      Logs_lwt.info(m =>
        m("Creating dir: %s from %s and %s", dir, project.output_dir, path)
      )
      >>= (() => Base.OS.mkdirp(dir));
    | `Compile(cunit) =>
      Logs_lwt.info(m =>
        m(
          "Compiling: %s => %s",
          cunit.input,
          Filename.concat(project.output_dir, cunit.output),
        )
      )
      >>= (() => Compiler.compile(project, cunit))
    }
  );

let execute_build = (project, cunits) => {
  let began_at = Unix.gettimeofday();
  let cunit_count = cunits |> List.length;
  Lwt.(
    Logs_lwt.debug(m =>
      m("About to process %d compilation units...", cunit_count)
    )
    >>= (
      () =>
        cunits
        |> List.filter(x =>
             switch (x) {
             | `Create_dir(_) => true
             | _ => false
             }
           )
        |> Lwt_list.iter_s(execute_rule(project))
        >>= (
          () =>
            cunits
            |> List.filter(x =>
                 switch (x) {
                 | `Compile(_) => true
                 | _ => false
                 }
               )
            |> List.map(execute_rule(project))
            |> Lwt.join
        )
    )
    >>= (
      () =>
        Logs_lwt.debug(m => {
          let finished_at = Unix.gettimeofday();
          m(
            {j|Processes %d compilation units in %.3f s|j},
            cunit_count,
            finished_at -. began_at,
          );
        })
    )
  );
};

/* TODO(@ostera): pass in root as a pasameter */
let build = (_flags, project_root, output_dir) => {
  let began_at = Unix.gettimeofday();
  let work =
    Lwt.(
      Logs_lwt.app(m => m({j|🌵 Compiling project... |j}))
      >>= (() => read_project(project_root, output_dir))
      >>= (
        project =>
          switch (project) {
          | Ok(project) => project |> plan_build >>= execute_build(project)
          | Error(err) =>
            Logs_lwt.err(m => err |> Errors.to_string |> m("ERROR: %s"))
          }
      )
      >>= (
        () =>
          Logs_lwt.app(m => {
            let finished_at = Unix.gettimeofday();
            let delta = finished_at -. began_at;
            let has_errors = Logs.err_count() > 0;
            let msg =
              if (has_errors) {{j|💀 Failed in|j}} else {{j|🌮 Done in|j}};
            m("%s %0.3fs", msg, delta);
          })
      )
    );

  Lwt_main.run(work);
};
