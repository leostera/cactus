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

  let isdir = path => Dir.exists(path) |> R.ignore_error(~use=_ => false);

  let readdir = path =>
    Dir.contents(~rel=true, path) |> R.ignore_error(~use=_ => []);

  let writefile: (Fpath.t, string) => unit =
    (path, content) => File.write(~mode=0o777, path, content) |> ignore;

  let readfile: Fpath.t => string =
    path => File.read(path) |> R.ignore_error(~use=_ => "");

  let mkdirp: Fpath.t => unit =
    path => Dir.create(~path=true, ~mode=0o777, path) |> ignore;
};
