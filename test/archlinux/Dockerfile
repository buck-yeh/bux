FROM archlinux/archlinux
USER root
#ENV PACDAY='2019/05/07'
RUN cat <<EOF | tee /etc/pacman.d/mirrorlist
Server = http://mirror.archlinux.tw/ArchLinux/\$repo/os/\$arch
Server = https://mirror.archlinux.tw/ArchLinux/\$repo/os/$arch
Server = http://archlinux.ccns.ncku.edu.tw/archlinux/\$repo/os/\$arch
Server = http://mirror.rackspace.com/archlinux/\$repo/os/\$arch
Server = https://mirror.rackspace.com/archlinux/\$repo/os/\$arch
EOF
RUN if [[ -n "$PACDAY" ]]; then \
        sed -Ei "s|^Include\s*=.+$|SigLevel = PackageRequired\nServer=https://archive.archlinux.org/repos/$PACDAY/\$repo/os/\$arch|g" /etc/pacman.conf ; \
    fi
RUN sed -Ei 's|^#VerbosePkgLists\s*$|VerbosePkgLists|g' /etc/pacman.conf
RUN pacman -Sy --needed --noconfirm cmake make gcc git binutils fakeroot debugedit gawk catch2 glibc ncurses sudo
RUN rm -rf /root/.cache
RUN useradd -m guest

USER guest
RUN  echo $'\
PS1=\"\\[\\033[01;37m\\]\\D{%m/%d %H:%M:%S} \\[\\033[01;32m\\]\\u@\\h\[\\033[00m\\]:\\[\\033[01;43m\\]\\w\\[\\033[00m\\] \"\n\
alias ll=\'ls -lF --color=auto --time-style=\"+%y/%m/%d %H:%M:%S\"\'\n\
export LANG=en_US.UTF-8\n\
locale-gen\n' >> ~/.bashrc
COPY --chown=guest:guest PKGBUILD /home/guest/Duty/
COPY --chown=guest:guest aur_poc/* /home/guest/Duty/aur_poc/
RUN  cd ~/Duty && \
     makepkg -s
WORKDIR /home/guest/Duty

USER root
RUN  pacman -U --noconfirm /home/guest/Duty/bux-*.pkg.tar.*
USER guest
RUN  cd ~/Duty/aur_poc && \
     cmake . && \
     make && \
     ctest .
