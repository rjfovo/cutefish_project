1. 配置多个用户
[user]
	name = xxxxx
	email = xxxxxx@qq.com

[includeIf "gitdir:/workspace/cutefish_project/"] # 目录一定要以 "/" 结尾
	path = ~/.gitconfig-cutefish
	[core]
    		sshCommand = "ssh -i /root/.ssh/cutefish_rsa" # 其他用户使用的私钥