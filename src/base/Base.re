module L = {
  let buckets: (int, list('a)) => list(list('a)) =
    (count, ls) => {
      let len = ls |> List.length |> float_of_int;
      let count = count |> float_of_int;
      let size = len /. count |> ceil |> int_of_float;
      let rec part = (acc, n, l) =>
        switch (n, l) {
        | (0, _)
        | (_, []) => (acc, l)
        | (n, [x, ...xs]) => part([x, ...acc], n - 1, xs)
        };
      let rec buck = (acc, l) =>
        switch (l) {
        | [] => acc
        | xs =>
          let (b, l') = part([], size, xs);
          buck([b, ...acc], l');
        };
      buck([], ls);
    };
};
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
  open Bos.OS;
  module R = Rresult.R;

  let (<|>) = (a, b) =>
    switch (a) {
    | Ok(x) => x
    | _ => b
    };

  let exists = path => File.exists(path) <|> false;

  let isdir = path => Dir.exists(path) <|> false;

  let readdir = (~rel=false, path) =>
    Dir.contents(~rel, path) <|> [] |> List.map(Fpath.normalize);

  let splitdir = contents => {
    let (dirs, files) =
      contents |> List.partition(name => name |> Dir.exists <|> false);
    (`Dirs(dirs |> List.map(Fpath.to_dir_path)), `Files(files));
  };

  let writefile: (Fpath.t, string) => unit =
    (path, content) => File.write(~mode=0o777, path, content) |> ignore;

  let readfile: Fpath.t => string = path => File.read(path) <|> "";

  let mkdirp: Fpath.t => unit =
    path => Dir.create(~path=true, ~mode=0o777, path) |> ignore;

  module Async = {
    open Lwt.Infix;

    let mkdirp = path =>
      Lwt.catch(() => Lwt_unix.mkdir(path, 0o0777), _ => Lwt.return_unit);

    let writefile = (path, contents) =>
      Lwt_io.with_file(~mode=Lwt_io.Output, path, chan =>
        Lwt_io.write(chan, contents)
      );

    let readfile = path =>
      Lwt_io.with_file(~mode=Lwt_io.Input, path, ch =>
        ch |> Lwt_io.read_lines |> Lwt_stream.to_list >|= String.concat("\n")
      );
  };
};
