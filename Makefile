all: push
commit:
	git commit -a || true
push: commit
	git push master || true

