" Use indentation scripts located in the indent folder of your VIM installation.
if has("autocmd")
  " Enable file type detection.
  " Use the default filetype settings, so that mail gets 'tw' set to 72,
  " 'cindent' is on in C files, etc.
  " Also load indent files, to automatically do language-dependent indenting.
"  filetype plugin indent on
endif

colorscheme evening

" Turn on color syntax highlighting.
syntax on

" Special case when in diff mode.
if &diff
    syntax off
endif

" Show whitespace.
set list
set listchars=eol:$,tab:>-,trail:~,extends:>,precedes:<

" Enable spell check.
set spell spelllang=en_us
hi clear SpellBad
hi SpellBad cterm=underline

" Enable line numbers.
set number

" set autoindent
set tabstop=4
set shiftwidth=4
set expandtab

" Put a red mark when you go over 100 columns.
" The matchadd is a reg ex - the v is virtual.
highlight ColorColumn ctermbg=red
call matchadd('ColorColumn', '\%101v', 100)

" Highlight all search matches.
set hlsearch
hi MatchParen cterm=underline ctermbg=Black ctermfg=Red

" Press F4 to toggle highlighting on/off, and show current value.
:noremap <F4> :set hlsearch! hlsearch?<CR>

" Press return to temporarily get out of the highlighted search.
:nnoremap <CR> :nohlsearch<CR><CR>

" Put swap, backup and undo files in a special location instead of the working directory of the
" file being edited
set backup
set backupdir=~/.vim/backup//
set directory=~/.vim/swap//
set undodir=~/.vim/undo//
set writebackup

" Look in directory containing current file (.),
" then current directory (empty text between two commas),
" then each directory under current directory ('**').
:set path=.,,**

" Use Tab and Shift+Tab to go to next and previous tabs.
:nnoremap <Tab> :tabn<CR>
:nnoremap <S-Tab> :tabp<CR>

" Support for <Ctrl-Up>.
:nnoremap [A <C-Y>
:inoremap [A <Esc><C-Y>i

" Support for <Ctrl-Down>.
:nnoremap [B <C-E>
:inoremap [B <Esc><C-E>i

" Use <Ctrl-Right> to jump forward one word.
:nnoremap [C w
:inoremap [C <C-O>w

" Use <Ctrl-Left> to jump back one word.
:nnoremap [D b
:inoremap [D <C-O>b

:nnoremap <C-Home> gg
:inoremap <C-Home> <Esc>ggi

:nnoremap <C-End> G
:inoremap <C-End> <Esc>G<End>i