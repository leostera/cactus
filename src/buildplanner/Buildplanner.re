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
            |> List.map(Parser.read_site),
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
       Compiler.Rules.{
         input: Filename.concat(site.dir, filename),
         output: Filename.concat(site.dir, output_name),
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
           let fullpath = Filename.concat(project.output_dir, site.dir);
           Buildgraph.Node(
             `Create_dir(fullpath),
             site
             |> find_docs
             |> List.map(cunit => Buildgraph.Leaf(`Compile(cunit))),
           );
         }),
    );
