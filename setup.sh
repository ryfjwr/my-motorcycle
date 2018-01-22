#!/bin/bash

sudo yum install -y git
sudo yum install -y gcc make openssl-devel libffi-devel readline-devel git sqlite sqlite-devel
git clone https://github.com/sstephenson/rbenv.git ~/.rbenv

echo 'export PATH="$HOME/.rbenv/bin:$PATH"' >> ~/.bash_profile
echo 'eval "$(rbenv init -)"' >> ~/.bash_profile
source ~/.bash_profile

mkdir ~/.rbenv/plugins
git clone https://github.com/sstephenson/ruby-build.git ~/.rbenv/plugins/ruby-build
rbenv install 2.3.6
rbenv global 2.3.6
~/.rbenv/shims/gem install bundler
~/.rbenv/shims/rbenv rehash
