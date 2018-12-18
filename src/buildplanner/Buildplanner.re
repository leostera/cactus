open Model;

let find_sites = root => {
  let rec crawl = (sites, top) =>
    if (top |> Base.OS.isdir) {
      let files =
        switch (top |> Base.OS.readdir) {
        | [] => []
        | files =>
          List.concat([
            /* check if any of the current files are site files */
            files
            |> List.filter(name => Fpath.equal(name, Model.site_filename))
            |> List.map(Fpath.append(top))
            |> List.filter(path => Base.OS.isdir(path) == false)
            |> List.map(Parser.read_site),
            /* keep crawling the file tree */
            files
            |> List.map(Fpath.append(top))
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

let find_docs = site =>
  site.dir
  |> Base.OS.readdir
  |> List.filter(filename => {
       let fullpath = Fpath.append(site.dir, filename);
       ! (fullpath |> Base.OS.isdir);
     })
  |> List.filter(filename => {
       let ext = filename |> Fpath.get_ext;
       ext == ".md";
     })
  |> List.map(filename => {
       let output_name = filename |> Fpath.rem_ext |> Fpath.add_ext(".html");
       Compiler.Rules.{
         input: Fpath.append(site.dir, filename),
         output: Fpath.append(site.dir, output_name),
       };
     });

let plan_build:
  Model.project => Buildgraph.plan(Compiler.Rules.compilation_unit) =
  project =>
    Buildgraph.Node(
      `Create_dir(project.output_dir),
      project.root
      |> find_sites
      |> List.map(site => {
           let fullpath = Fpath.append(project.output_dir, site.dir);
           Buildgraph.Node(
             `Create_dir(fullpath),
             site
             |> find_docs
             |> List.map(cunit => Buildgraph.Leaf(`Compile(cunit))),
           );
         }),
    );
