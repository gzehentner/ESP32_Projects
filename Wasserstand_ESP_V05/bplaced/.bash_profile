export PS1='\[\033[01;31m\]$USER\[\033[01;33m\]@\[\033[01;36m\]\h \[\033[01;33m\]\w \[\033[01;35m\]\$ \[\033[00m\]'
umask 072

alias ls='ls $LS_OPTIONS'
alias ll='ls $LS_OPTIONS -l'
alias l='ls $LS_OPTIONS -lA'
alias ..='cd ..'
alias ...='cd ../..'
alias s='ssh -l root'

eval "`dircolors`"
export EDITOR="vim"
export HISTFILESIZE=99999999
export HISTSIZE=99999999
export HISTCONTROL="ignoreboth"
export LS_OPTIONS='--color=auto -h'

if [[ -O /users/_temp/$USER/ && -d /users/_temp/$USER/ ]]; then
    TMPDIR=/users/_temp/$USER/
    TMP=$TMPDIR
    TEMP=$TMPDIR
    export TMPDIR TMP TEMP
fi

echo -e "\n\nWillkommen bei \033[1;34mb\033[1;37mplaced\033[39m!\n"
