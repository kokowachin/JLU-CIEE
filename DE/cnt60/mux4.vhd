library ieee;
use ieee.std_logic_1164.all;

entity mux4 is
    port (
        sel    : in  std_logic_vector(1 downto 0);
        num0   : in  std_logic_vector(6 downto 0);  -- 个位
        num1   : in  std_logic_vector(6 downto 0);  -- 十位
        num2   : in  std_logic_vector(6 downto 0);  -- 百位
        num3   : in  std_logic_vector(6 downto 0);  -- 千位
        numout : out std_logic_vector(6 downto 0)
    );
end mux4;

architecture bh of mux4 is
begin
    with sel select
        numout <= num0 when "00",
                  num1 when "01",
                  num2 when "10",
                  num3 when "11",
                  "1111111" when others;  -- 全灭（安全）
end bh;