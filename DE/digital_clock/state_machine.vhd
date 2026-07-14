library ieee;
use ieee.std_logic_1164.all;

entity state_machine is
    port (
        clk_1khz      : in  std_logic;                     -- 系统同步时钟
        key_pulse     : in  std_logic_vector(1 downto 0);  -- 消抖后的按键脉冲: (0)为Mode切换, (1)为Add增加
        alarm_ringing : in  std_logic;                     -- 闹铃正在响的标志 (来自闹钟模块)
        
        state_out     : out integer range 0 to 5;          -- 输出当前状态给其他模块
        
        -- 分发给计时/闹钟模块的控制脉冲 (高电平有效，仅1个时钟周期宽)
        add_t_hr      : out std_logic;                     -- 调时: 小时+1
        add_t_min     : out std_logic;                     -- 调时: 分钟+1
        add_a_hr      : out std_logic;                     -- 调闹钟: 小时+1
        add_a_min     : out std_logic;                     -- 调闹钟: 分钟+1
        toggle_12_24  : out std_logic;                     -- 切换12/24小时制
        stop_alarm    : out std_logic                      -- 停止闹钟脉冲
    );
end state_machine;

architecture rtl of state_machine is
    -- 状态编码定义
    -- 0: 正常运行 (Normal)
    -- 1: 调时 - 小时 (Set Time Hour)
    -- 2: 调时 - 分钟 (Set Time Min)
    -- 3: 调闹钟 - 小时 (Set Alarm Hour)
    -- 4: 调闹钟 - 分钟 (Set Alarm Min)
    -- 5: 12/24制切换 (Toggle Format)
    signal current_state : integer range 0 to 5 := 0;
    
begin
    process(clk_1khz)
    begin
        if rising_edge(clk_1khz) then
            -- 默认状态：所有控制脉冲拉低
            -- 这样可以保证加法指令只维持一个时钟周期，防止连续狂加
            add_t_hr     <= '0';
            add_t_min    <= '0';
            add_a_hr     <= '0';
            add_a_min    <= '0';
            toggle_12_24 <= '0';
            stop_alarm   <= '0';
            
            ---------------------------------------------------
            -- 优先级最高：如果闹钟正在响，按任意键只负责关闹钟
            ---------------------------------------------------
            if alarm_ringing = '1' then
                if key_pulse(0) = '1' or key_pulse(1) = '1' then
                    stop_alarm <= '1';
                end if;
                
            ---------------------------------------------------
            -- 正常按键逻辑
            ---------------------------------------------------
            else
                -- 处理 KEY0 (MODE 切换键)
                if key_pulse(0) = '1' then
                    if current_state = 5 then
                        current_state <= 0; -- 状态 5 之后回到 0 (循环)
                    else
                        current_state <= current_state + 1;
                    end if;
                end if;
                
                -- 处理 KEY1 (ADD 增加键)
                if key_pulse(1) = '1' then
                    case current_state is
                        when 1 => add_t_hr     <= '1';  -- 在状态1下按ADD，输出调时(小时)脉冲
                        when 2 => add_t_min    <= '1';  -- 在状态2下按ADD，输出调时(分钟)脉冲
                        when 3 => add_a_hr     <= '1';  -- 在状态3下按ADD，输出调闹钟(小时)脉冲
                        when 4 => add_a_min    <= '1';  -- 在状态4下按ADD，输出调闹钟(分钟)脉冲
                        when 5 => toggle_12_24 <= '1';  -- 在状态5下按ADD，切换显示格式
                        when others => null;            -- 在状态0(正常显示)下按ADD，不产生任何操作
                    end case;
                end if;
            end if;
        end if;
    end process;
    
    -- 将内部状态输出，后续数码管显示模块需要依靠这个状态来决定“谁该闪烁”
    state_out <= current_state;
    
end rtl;