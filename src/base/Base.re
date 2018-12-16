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
  let file_exists = path =>
    path
    |> Lwt_unix.stat
    |> Lwt.map(s =>
         Unix.(
           switch (s.st_kind) {
           | S_REG => true
           | _ => false
           }
         )
       );

  let writefile = (path, contents) =>
    Lwt_io.with_file(~mode=Lwt_io.Output, path, chan =>
      Lwt_io.write(chan, contents)
    );

  let concatall = (~init="", sep, str) =>
    Lwt_stream.fold((a, b) => a ++ sep ++ b, str, init);

  let readlines = chan => chan |> Lwt_io.read_lines |> concatall("\n");

  let readfile = path =>
    Lwt_io.with_file(~mode=Lwt_io.Input, path, readlines);

  let isdir = path =>
    Lwt.catch(
      () =>
        path
        |> Lwt_unix.stat
        |> Lwt.map(s =>
             Unix.(
               switch (s.st_kind) {
               | S_DIR => true
               | _ => false
               }
             )
           ),
      _ => Lwt.return(false),
    );

  let rec readdir = path =>
    Lwt_unix.files_of_directory(path)
    |> Lwt_stream.map_s(name => {
         let relpath = Filename.concat(path, name);
         isdir(relpath)
         |> Lwt.map(v =>
              switch (v, name) {
              | (true, ".")
              | (true, "..") => Lwt_stream.of_list([])
              | (true, _) => readdir(relpath)
              | (_, _) => Lwt_stream.of_list([relpath])
              }
            );
       })
    |> Lwt_stream.concat;

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
    |> Lwt_list.iter_s(mkpath =>
         Lwt.catch(() => Lwt_unix.mkdir(mkpath, 0o777), _ => Lwt.return())
       );
  };
};
