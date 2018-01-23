require 'sinatra/base'
require 'sinatra/reloader'
require 'sinatra/json'
require 'sequel'
require 'pry'
require 'haml'
require_relative './common.rb'

class App < Sinatra::Base

  def gmap_converter(attr)
    begin
      degree_with_per = (attr / 100.0)
      # 小数
      converted_degree = Float(degree_with_per.to_s.split('.').last) / 60.0
      num = 10 ** converted_degree.to_s.split('.').first.length
      degree_with_per.floor + (converted_degree / num)
    rescue => e
    end
  end

  set :bind, '0.0.0.0'
  configure :development do
    register Sinatra::Reloader
  end

  get '/' do
    haml :index
  end

  get '/places' do
    places =
      $DB[:places].all.map do |place|
        {id: place[:id], lat: gmap_converter(place[:lat]), lng: gmap_converter(place[:lng]), created_at: place[:created_at]}
      end
    json places
  end

  post '/' do
    lat = params['lat']
    lng = params['lng']

    $DB[:places].insert(lat: lat, lng: lng, created_at: Time.now)
    status 200
  end

  run!
end


