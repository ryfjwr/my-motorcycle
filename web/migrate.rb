require 'sequel'
require_relative './common.rb'

$DB.create_table :places do
  primary_key :id
  Float :lat
  Float :lng
  Time :created_at
end
