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
    let doc = "build your project";
    let exits = Term.default_exits;
    let man = [
      `S(Manpage.s_description),
      `P({j|asdf|j}),
      `Blocks(SharedOpts.help),
    ];

    (
      Term.(const(Cactus.build) $ SharedOpts.flags),
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
    Term.info("cactus", ~version="0.1.0", ~doc, ~sdocs, ~exits, ~man),
  );
};

let cmds = [Build.cmd];

Term.(exit @@ eval_choice(default_cmd, cmds));
