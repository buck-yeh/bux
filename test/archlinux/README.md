# How to test a upstream commit and publish it to [AUR](https://aur.archlinux.org/)

1. Copy everything from this folder to a new folder and `cd` to it in a terminal.
2. In file `PKGBUILD`, edit `$pkgver` to the branch or tag name you choose to test against and save it

   ~~~PKGBUILD
   pkgver=main
   ~~~

3. Make an AUR tarball

   ~~~bash
   makepkg -s
   ~~~

   If you find a new file name like `bux-main-1-x86_64.pkg.tar.zst`, it's a success !<br>
   💡 The `main` part is replaced by any tag name you assigned to `$pkgver`.

4. Make sure your docker server is up, and execute

   ~~~bash
   ./docker_build
   ~~~

   If the last part of terminal output looks like the below, it's a success! Otherwise, it fails!

   ~~~bash
   .... (skipped)
   12/14 Test #12: test_ezargs_All .........................   Passed    0.00 sec
       Start 13: test_logger_All
   13/14 Test #13: test_logger_All .........................   Passed    0.00 sec
       Start 14: test_paralog_All
   14/14 Test #14: test_paralog_All ........................   Passed    0.00 sec

   100% tests passed, 0 tests failed out of 14

   Total Test time (real) =   0.04 sec
   Removing intermediate container bf10d798f856
   ---> 49a45b388b54
   Successfully built 49a45b388b54
   ~~~

5. Now you can create a new release [upstream](https://github.com/buck-yeh/bux/releases)
6. Copy `PKGBUILD` to the [AUR downstream working folder](https://wiki.archlinux.org/title/AUR_submission_guidelines), for example mine is in `~/AUR/bux.git`.
7. With the release/tag name, say `1.6.4`, edit `$pkgver` in `PKGBUILD`

   ~~~PKGBUILD
   pkgver=1.6.4
   ~~~

8. Then execute

   ~~~bash
   makepkg --printsrcinfo > .SRCINFO
   ~~~

9. Just give it a `git push` and we are done !