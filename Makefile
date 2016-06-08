all: push
commit:
	git commit -a
push: commit
	git push master

