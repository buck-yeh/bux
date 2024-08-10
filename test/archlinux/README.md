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
   💡 `main` can be replaced by any existing release/tag name.

4. Make sure your docker server is up, and execute

   ~~~bash
   sudo ./docker_build
   ~~~

   If the last part of terminal output looks like the below, it's a success! Otherwise, it fails!

   ~~~
   .... (skipped)
   #18 30.43 12/14 Test #12: test_ezargs_All .........................   Passed    0.00 sec
   #18 30.43       Start 13: test_logger_All
   #18 30.43 13/14 Test #13: test_logger_All .........................   Passed    0.00 sec
   #18 30.43       Start 14: test_paralog_All
   #18 30.43 14/14 Test #14: test_paralog_All ........................   Passed    0.00 sec
   #18 30.43 
   #18 30.43 100% tests passed, 0 tests failed out of 14
   #18 30.43 
   #18 30.43 Total Test time (real) =   0.03 sec
   #18 DONE 30.5s

   #19 exporting to image
   #19 exporting layers
   #19 exporting layers 3.1s done
   #19 writing image sha256:ba0a168b30a90879e727bd806f5c8e3160d321bf446141e9fbf9dd7c46157947 done
   #19 DONE 3.1s
   ~~~

5. Now you can create a new release [upstream](https://github.com/buck-yeh/bux/releases)
6. Copy `PKGBUILD` to the [AUR downstream working folder](https://wiki.archlinux.org/title/AUR_submission_guidelines), for example mine is in `~/AUR/bux.git`.
7. With the release/tag name, say `1.7.0`, edit `$pkgver` in `PKGBUILD`

   ~~~PKGBUILD
   pkgver=1.7.0
   ~~~

8. Then execute

   ~~~bash
   makepkg --printsrcinfo > .SRCINFO
   ~~~

9. `git commit`, `git push`, and we are done !
