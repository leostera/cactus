open Model;

let find_sites = (~output_dir, root) => {
  let rec crawl = (sites, top) => {
    let is_dir = top |> Fpath.is_dir_path;
    let is_output_dir = Fpath.equal(output_dir, top);
    if (! is_output_dir && is_dir) {
      let (`Dirs(dirs), `Files(files)) =
        top |> Base.OS.readdir(~rel=false) |> Base.OS.splitdir;

      let sitefile = Fpath.append(top, Model.site_filename);
      let site_compilation_unit =
        switch (Base.OS.exists(sitefile), Parser.read_site(sitefile)) {
        | (true, Error(`Msg(msg))) =>
          Logs.err(m =>
            m("Error parsing %s: %s", sitefile |> Fpath.to_string, msg)
          );
          None;
        | (true, Ok(site)) =>
          Logs.debug(m =>
            m("Found site file at %s", sitefile |> Fpath.to_string)
          );
          /* check if any of the current files are docs */
          let (docs, other_files) =
            files |> List.partition(Fpath.has_ext(".md"));

          Logs.debug(m => {
            m("Found %d other files", other_files |> List.length);
            m("Found %d docs", docs |> List.length);
          });

          let fullpath = Fpath.append(output_dir, site.dir);
          let template =
            Base.Option.(
              site.template
              >>| (path => path |> Fpath.append(site.dir) |> Base.OS.readfile)
            );

          /* create a buildgraph node */
          Some(
            Buildgraph.Node(
              `Create_dir(fullpath),
              docs
              |> List.map(filename => {
                   let output_name =
                     filename |> Fpath.rem_ext |> Fpath.add_ext(".html");
                   let cunit =
                     Compiler.Rules.{input: filename, output: output_name};
                   let compile_cunit = `Compile(cunit);
                   switch (template) {
                   | None => Buildgraph.Leaf(compile_cunit)
                   | Some(template_contents) =>
                     let template_cunit =
                       Compiler.Rules.{
                         input: cunit.output,
                         output: cunit.output,
                         template: template_contents,
                       };
                     Buildgraph.Node(
                       compile_cunit,
                       [Buildgraph.Leaf(`Template(template_cunit))],
                     );
                   };
                 }),
            ),
          );
        | _ => None
        };

      /* keep crawling the file tree */
      let subdirs = dirs |> List.map(crawl([])) |> List.concat;

      switch (site_compilation_unit) {
      | None => subdirs @ sites
      | Some(cunit) => [cunit, ...subdirs @ sites]
      };
    } else {
      sites;
    };
  };
  crawl([], root);
};

let plan_build:
  Model.project => Buildgraph.plan(Compiler.Rules.compilation_unit) =
  project =>
    Buildgraph.Node(
      `Create_dir(project.output_dir),
      project.root |> find_sites(~output_dir=project.output_dir),
    );
