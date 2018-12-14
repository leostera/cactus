module Result = {
  let map: ('a => 'b, result('a, 'c)) => result('b, 'c) =
    (f, x) =>
      switch (x) {
      | Ok(value) => Ok(f(value))
      | error => error
      };

  let sequence: list(result('a, 'b)) => result(list('a), list('b)) =
    rs => {
      let rec seq = (rs, oks, errs) =>
        switch (rs) {
        | [] => (oks, errs)
        | [Ok(r), ...rs'] => seq(rs', [r, ...oks], errs)
        | [Error(e), ...rs'] => seq(rs', oks, [e, ...errs])
        };
      let (oks, errs) = seq(rs, [], []);
      switch (errs |> List.length) {
      | 0 => Ok(oks)
      | _ => Error(errs)
      };
    };

  module Infix = {
    let (>>=) = (x, f) => map(f, x);
  };
};

module OS = {
  let writefile = (path, contents) => {
    let chan = open_out(path);
    output_string(chan, contents);
    flush(chan);
    close_out(chan);
  };

  let readfile = path => {
    let chan = open_in(path);
    let rec read = acc =>
      switch (chan |> input_line) {
      | exception End_of_file => acc |> List.rev
      | line => read([line, ...acc])
      };
    let content = read([]) |> List.fold_left((++), "\n");
    close_in(chan);
    content;
  };

  let mkdirp = path => {
    let rec build_paths = (splits, acc) =>
      switch (splits) {
      | [] => acc |> List.rev
      | [x, ...xs] =>
        switch (acc) {
        | [] => build_paths(xs, [x, ...acc])
        | [prev, ..._] =>
          build_paths(xs, [Filename.concat(prev, x), ...acc])
        }
      };

    let split_char = Filename.dir_sep.[0];
    let splits = path |> String.split_on_char(split_char);

    build_paths(splits, [])
    |> List.iter(mkpath =>
         switch (Sys.is_directory(mkpath)) {
         | exception _ => Unix.mkdir(mkpath, 0o777)
         | _ => ()
         }
       );
  };
};
