raw = File.open(ARGV.first, "rb", encoding: Encoding::ASCII_8BIT).read
samples = raw.unpack("s*")

puts "#include <stdint.h>"
puts "extern const int16_t wavetable[8][8][256] = {"
  puts 8.times.map {
    "  {\n" + 8.times.map {
      "    {" + 256.times.map {
        samples.shift
      }.join(", ") + "}"
    }.join(",\n") + "  }"
  }.join(",\n");
puts "};"
