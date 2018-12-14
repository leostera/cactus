open Cmdliner;

module SharedOpts = {
  let help = [
    `S(Manpage.s_common_options),
    `P("These options are common to all commands."),
    `S("MORE HELP"),
    `P("Use `$(mname) $(i,COMMAND) --help' for help on a single command."),
    `Noblank,
    `S(Manpage.s_bugs),
    `P("Check bug reports at https://github.com/ostera/cactus."),
  ];
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
      Term.(const(Cactus.build) $ const()),
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
