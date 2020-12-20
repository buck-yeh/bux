FROM archlinux/archlinux
USER root
#ENV PACDAY='2019/05/07'
RUN echo $'\
Server = http://archlinux.cs.nctu.edu.tw/$repo/os/$arch\n\
Server = http://ftp.tku.edu.tw/Linux/ArchLinux/$repo/os/$arch\n' | tee /etc/pacman.d/mirrorlist && \
    if [[ -n "$PACDAY" ]]; then \
        sed -Ei "s|^Include\s*=.+$|SigLevel = PackageRequired\nServer=https://archive.archlinux.org/repos/$PACDAY/\$repo/os/\$arch|g" /etc/pacman.conf ; \
    fi && \
    sed -Ei 's|^#TotalDownload\s*$|TotalDownload|g' /etc/pacman.conf && \
    sed -Ei 's|^#VerbosePkgLists\s*$|VerbosePkgLists|g' /etc/pacman.conf && \
    pacman -Sy --needed --noconfirm cmake make gcc git binutils fmt fakeroot gawk && \
    rm -rf /root/.cache
RUN useradd -m guest

USER guest
RUN  echo $'\
PS1=\"\\[\\033[01;37m\\]\\D{%m/%d %H:%M:%S} \\[\\033[01;32m\\]\\u@\\h\[\\033[00m\\]:\\[\\033[01;43m\\]\\w\\[\\033[00m\\] \"\n\
alias ll=\'ls -lF --color=auto --time-style=\"+%y/%m/%d %H:%M:%S\"\'\n\
export LANG=en_US.UTF-8\n\
locale-gen\n' >> ~/.bashrc
COPY --chown=guest:guest PKGBUILD CMakeLists.txt test_expand_env.cpp test_in.txt /home/guest/Duty/
RUN  cd ~/Duty && \
     makepkg -s
WORKDIR /home/guest/Duty

USER root
RUN  pacman -U --noconfirm /home/guest/Duty/bux-*.pkg.tar.zst
USER guest
RUN  cmake . && \
     make && \
     ctest .
