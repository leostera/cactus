open Cmdliner;

type verbosity =
  | Quiet
  | Normal
  | Verbose;

module SharedOpts = {
  let setup_logger = (debug, verbosity) => {
    Fmt_tty.setup_std_outputs();
    Logs.set_reporter(Logs_fmt.reporter());
    let verbosity' =
      switch (verbosity) {
      | Quiet => None
      | Normal => Some(Logs.App)
      | Verbose => debug ? Some(Logs.Debug) : Some(Logs.Info)
      };
    Logs.set_level(verbosity');
  };
  let help = [
    `S(Manpage.s_common_options),
    `P("These options are common to all commands."),
    `S("MORE HELP"),
    `P("Use `$(mname) $(i,COMMAND) --help' for help on a single command."),
    `Noblank,
    `S(Manpage.s_bugs),
    `P("Check bug reports at https://github.com/ostera/bsdoc."),
  ];
  let flags = {
    let docs = Manpage.s_common_options;
    let debug = {
      let doc = "Give only debug output.";
      Arg.(value & flag & info(["debug"], ~docs, ~doc));
    };

    let verb = {
      let doc = "Suppress informational output.";
      let quiet = (Quiet, Arg.info(["q", "quiet"], ~docs, ~doc));
      let doc = "Give verbose output.";
      let verbose = (Verbose, Arg.info(["v", "verbose"], ~docs, ~doc));
      Arg.(last & vflag_all([Normal], [quiet, verbose]));
    };

    Term.(const(setup_logger) $ debug $ verb);
  };
};

module Build = {
  let cmd = {
    let project_root = {
      let doc = "Root directory from where to start looking for nested site
      files.";
      Arg.(
        value & opt(file, "./") & info(["project-root"], ~docv="ROOT", ~doc)
      );
    };

    let output_dir = {
      let doc = "Output directory.";
      Arg.(
        value
        & opt(string, "./_public")
        & info(["o", "output-dir"], ~docv="OUTDIR", ~doc)
      );
    };

    let job_count = {
      let doc = "Amount of parallel jobs to run";
      Arg.(value & opt(int, 4) & info(["j", "jobs"], ~docv="JOBS", ~doc));
    };

    let doc = "build your project";
    let exits = Term.default_exits;
    let man = [
      `S(Manpage.s_description),
      `P(
        {j|Cactus will look for a cactus-project file in the \$ROOT folder
        (defaults to the current folder) and scan for site files to make a
        build plan, and execute it using $JOBS workers.|j},
      ),
      `P(
        {j|It will compile all of the markdown files it finds into .html
      files, copying the structure in which they are laid out in the file
      system.|j},
      ),
      `P(
        {j|That is, a file located in \$ROOT/pages/about-me.md will end up as
      \$OUTDIR/pages/about-me.html — as long as there is a SITE file in
      \$ROOT/pages.|j},
      ),
      `Blocks(SharedOpts.help),
    ];

    (
      Term.(
        const(Cactus.build)
        $ SharedOpts.flags
        $ project_root
        $ output_dir
        $ job_count
      ),
      Term.info("build", ~doc, ~sdocs=Manpage.s_common_options, ~exits, ~man),
    );
  };
};

let default_cmd = {
  let doc = "A Composable Static Site Generator";
  let sdocs = Manpage.s_common_options;
  let exits = Term.default_exits;
  let man = SharedOpts.help;
  (
    Term.(ret(const(`Help((`Pager, None))))),
    Term.info("cactus", ~version="0.3", ~doc, ~sdocs, ~exits, ~man),
  );
};

let cmds = [Build.cmd];

Term.(exit @@ eval_choice(default_cmd, cmds));
