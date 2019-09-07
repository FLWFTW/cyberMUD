if &cp | set nocp | endif
map \c :echo g:colors_name
map \p :CP
map \n :CN
let s:cpo_save=&cpo
set cpo&vim
nmap gx <Plug>NetrwBrowseX
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
map <F1> :TlistToggle
map <F12> :close
map <F8> :make
map <F7> :cnext
map <F6> :cprev
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set backspace=indent,eol,start
set backupdir=~/.vim/backups//
set cmdheight=2
set confirm
set directory=~/.vim/swaps//
set expandtab
set fileencodings=ucs-bom,utf-8,latin1
set guicursor=n-v-c:block,o:hor50,i-ci:hor15,r-cr:hor30,sm:block,a:blinkon0
set helplang=en
set hidden
set hlsearch
set ignorecase
set laststatus=2
set mouse=a
set ruler
set shiftwidth=3
set showcmd
set showmatch
set smartcase
set softtabstop=3
set nostartofline
set statusline=%-3.3n\ %f\ %h%m%r%w[%{strlen(&ft)?&ft:'none'},%{strlen(&fenc)?&fenc:&enc},%{&fileformat}]%=%{synIDattr(synID(line('.'),col('.'),1),'name')}\ %b,0x%-8B\ %-14.(%l,%c%V%)\ %<%P
set visualbell
set wildmenu
" vim: set ft=vim :
