# Golang Chat Server

> W.I.P
> 
> Doc is [here](https://www.thepolyglotdeveloper.com/2016/12/create-real-time-chat-app-golang-angular-2-websockets/)


## Something about Golang

Installation Directory: `/usr/local/go` in Unix. This is where Go's command-line tools, standard library and compiler lives. [Generally]

Workspace

### [Package, deprecated](https://medium.com/rungo/everything-you-need-to-know-about-packages-in-go-b8bac62b74cc)

> GOPATH is a big deal
> > When you install any dependency packages using go get command, it saves the package files under $GOPATH/src path. A Go program cannot import a dependency unless it is present inside $GOPATH. Also, go build command creates binary executable files and package archives inside $GOPATH. Hence, GOPATH is a big deal in Go. [always have to change $GOPATH, hard to maintain dependencies of different versions]
> > standard Go packages => GOROOT, other project-level dependencies => GOPATH
> > 与npm相比，npm的所有包都在project folder`node_modules`中，并使用package.json维护依赖的配置。


package name(the name of the directory), package declaration(first line of the code). 在package之间相互引用时，使用的是pkg declaration.

package name convention: 不加underscores, hyphens, maxiedCap

GOROOT, GOPATH, GOBIN

go 可以找到入口文件在哪里

package scope. Not allowed to redeclare a global variable with the same name in the same package. （不清楚子package是否能调用上级package）在全局作用域中初始化，go 会 计算initialization cycles. 当然也会找到initialization loop, 需要程序员避免。

`init` function. 不能显式调用，可以有多个`init()`，调用顺序会根据出现顺序（不同文件中的`init()`, 按字典序顺序调用）。

全局变量，要大写。其他包可以使用。

nested package 并不会在compiler父亲时也被递归地compiler, 需要指明。（父包与子包的scope是什么关系？）

```
go run *.go
├── Main package is executed
├── All imported packages are initialized
|  ├── All imported packages are initialized (recursive definition)
|  ├── All global variables are initialized 
|  └── init functions are called in lexical file name order
└── Main package is initialized
   ├── All global variables are initialized
   └── init functions are called in lexical file name order
```

`init`执行后, `main` function is called. Therefore, `init()` is used to initialize global variables (that cannot be initialized in the global context, like an array).

package with same name => package alias

`_` => null. want to initialize a package but not to use it. (maybe we just want to use a subpackage)

Dot import, community is not very fond of it.

在使用go get时，会去对应url找相应内容（尝试不同的VCS，尝试不同的网络协议）（根据`\<meta name="go-import" ...>` tag）中间可能会redirect。尽量不去redirect，保证go get <url> 等于仓库所在位置。

> 问题，there is no way to install a pkg pointed to a apecific Git version. cannot save multiple versions of the same pkg at the same time.

### [Anatomy of Modules in Go](https://medium.com/rungo/anatomy-of-modules-in-go-c8274d215c16)

Go Package, Old => Go Module, new. More about Go workspace.

module => package of packages / nested package... must be a VCS repository, should contain one or more `.go` files.

```
go mod init <import-path>  // like package.json for npm
```

create modules inside $GOPATH is disabled by default.

Semantic Versioning: v.X.Y.Z: vMajor.Minor.Patch

> [additional pre-release tags](https://drupal.stackexchange.com/questions/99612/what-does-rc-stand-for-when-to-use-alpha-beta-and-dev-instead): -rc.0 / beta.1
> 
> 1. rc for release candidate, deemed suitable by the author for production sites. [all critical bug type issues are reported fixed]
> 2. unstable
> 3. alpha, Most reported errors are resolved, but there may still be serious outstanding known issues, including security issues. Not thoroughly tested.
> 4. beta, beta: All critical data loss and security bugs are resolved. If the module offers an API, it should be considered frozen, so that those using the the API can start upgrading their projects.

Go 要求，不兼容性只体现在major release之间，并视不同的major release为different module. 我们要tagging for commit hash. 暂使用lightweight tags.

// not found: github.com/Hughshine/learngo@v1.0.0: invalid version: unknown revision v1.0.0

出现了 unknown revision 问题，根据这个[issue](https://github.com/golang/go/issues/36624)临时解决。
设置 `export GOPROXY=direct`，`export GOSUMDB=off` 

在go run时，`go.mod`中自动`require`了最新版本的module. 还多了一个`go.sum`文件，它可以ensure 100% reproducible builds(但它不是lock file)

1. When Go will install the module automatically? When we execute the command go run or other Go commands like test, build, Go automatically checks the third party import statements (like our module here), and clones the repository inside module cache directory.
2. What’s the use of go get then? 
3. Where are the modules stored on the system? Go modules are stored inside $GOPATH/pkg/mod directory (module cache directory)

小patch/minor version的更新：To update Go modules present inside a module with go.mod file, use go get -u command. This command will update all the modules with the latest minor or patch version of the given major version (explained later).

```
go get -u
go get -u=patch
go get module@version
```

如果要升级一个大版本：必须手动go get and specify the new import path

********************************

direct / indirect dependencies 的区分。

by default, the version suffix is v1. 如果`go.mod`中没有指明版本。

而对于只有minor/patch不同的release，无法同时import => Diamond Dependency Problem, 两个依赖，各自依赖一个只相差minor的另一个module。此时Go会选择最大版本（Minimal Version Selection）, 这要求在同一个major版本下，代码是向后兼容的。


****************************************************************

local development. 相同module下不同package之间的依赖：直接写完整的在线module path即可；不同module之间的依赖，在`go.mod`中使用`replace` directive. **在commit前要删除**

**** 

`go mod tidy` If you have a module that you wish to publish but its go.mod file does not track the dependencies that might have been imported inside the source code then you should run go mod tidy command.

 This command adds dependencies to the go.mod file if they are imported inside the source code and remove dependencies from go.mod if they are not used in the source code. So ideally, you should always run this command before publishing the module.

 The ./... pattern matches all the packages in a module. So when you run go build ./... or go test ./..., you are building or testing all the packages inside a module (including module itself if it is a package). These commands also download and install dependencies inside go.mod file.

 The go mod graph shows you the dependencies of your module.

对依赖的修正、检查。

*** 

强制将dependency先缓存下来

Sometimes, when you are running automated tests for your project (executable module like main in our case), there is a chance that your test machine may face some network issues while downloading the dependencies or when a test machine is running in an isolated environment.
In such cases, you need to provide dependencies beforehand. This is called vendoring. You can use the command go mod vendor to output all the dependencies inside vendor directory (inside module directory).
When you use go build, it looks for the dependencies inside module cache directory but you can force Go to use dependencies from vendor directory of the module using the command go build -mod vendor.

*** 

To install a CLI application (module) from anywhere in the terminal, use the following command.
$ GO111MODULE=on go install <package>@<version>
This will install a module inside the module cache directory and generate a binary executable file inside $GOPATH/bin directory.