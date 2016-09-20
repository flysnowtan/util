execute pathogen#infect()
syntax on
filetype plugin indent on

nmap <c-j> <c-w><c-j>
nmap <c-h> <c-w><c-h>
nmap <c-l> <c-w><c-l>
nmap <c-k> <c-w><c-k>

autocmd BufReadPost *
	\ if line("\"")>0&&line("\"")<=line("$") |
	\ exe "normal g'\"" | 
	\endif

map <C-n> :NERDTree<CR>

let g:miniBufExplMaxSize = 2

let Tlist_Show_One_File=1    " 只展示一个文件的taglist
let Tlist_Exit_OnlyWindow=1  " 当taglist是最后以个窗口时自动退出
let Tlist_Use_Right_Window=1 " 在右边显示taglist窗口
let Tlist_Sort_Type="name"   " tag按名字排序"

map :Tlist :Tlistoggle

filetype plugin indent on
set completeopt=longest,menu
let OmniCpp_NamespaceSearch = 2     " search namespaces in the current buffer   and in included files
let OmniCpp_ShowPrototypeInAbbr = 1 " 显示函数参数列表
let OmniCpp_MayCompleteScope = 1    " 输入 :: 后自动补全
let OmniCpp_DefaultNamespaces = ["std", "_GLIBCXX_STD"]

" 配置ctags的命令
" ctags -R --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q --language-force=C++

set autoindent
set number
set tabstop=4
set softtabstop=4
set shiftwidth=4
set history=1000

"YouCompleteMe

"设置error和warning的提示符，如果没有设置，ycm会以syntastic的
"" g:syntastic_warning_symbol 和 g:syntastic_error_symbol 这两个为准
let g:ycm_error_symbol='>>'
let g:ycm_warning_symbol='>*'

"设置跳转的快捷键，可以跳转到definition和declaration
nnoremap <leader>gc :YcmCompleter GoToDeclaration<CR>
nnoremap <leader>gf :YcmCompleter GoToDefinition<CR>
nnoremap <leader>gg :YcmCompleter GoToDefinitionElseDeclaration<CR>
"nmap <F4> :YcmDiags<CR>

"设置全局配置文件的路径
let g:ycm_global_ycm_extra_conf ='~/.vim/bundle/YouCompleteMe/third_party/ycmd/cpp/ycm/.ycm_extra_conf.py'
""开启基于tag的补全，可以在这之后添加需要的标签路径
let g:ycm_collect_identifiers_from_tags_files = 1
"开启语义补全
let g:ycm_seed_identifiers_with_syntax = 1
""在接受补全后不分裂出一个窗口显示接受的项
set completeopt-=preview
"不显示开启vim时检查ycm_extra_conf文件的信息
let g:ycm_confirm_extra_conf=0
""每次重新生成匹配项，禁止缓存匹配项
let g:ycm_cache_omnifunc=0
"在注释中也可以补全
let g:ycm_complete_in_comments=1
""输入第一个字符就开始补全
let g:ycm_min_num_of_chars_for_completion=1
"不查询ultisnips提供的代码模板补全，如果需要，设置成1即可
let g:ycm_use_ultisnips_completer=0

