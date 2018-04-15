﻿using CulverinEditor;
using CulverinEditor.Debug;

public class DaenerysCD_Left : CoolDown
{
    public int max_charges = 3;
    private int current_charges = 3;

    public GameObject charge_count_0;
    public GameObject charge_count_1;
    public GameObject charge_count_2;
    public GameObject charge_count_3;

    public GameObject daenerys_left_cd_text;

    void Start()
    {
        charge_count_0 = GetLinkedObject("charge_count_0");
        charge_count_1 = GetLinkedObject("charge_count_1");
        charge_count_2 = GetLinkedObject("charge_count_2");
        charge_count_3 = GetLinkedObject("charge_count_3");

        charge_count_0.GetComponent<CompImage>().SetEnabled(false, charge_count_0);
        charge_count_1.GetComponent<CompImage>().SetEnabled(false, charge_count_1);
        charge_count_2.GetComponent<CompImage>().SetEnabled(false, charge_count_2);
        charge_count_3.GetComponent<CompImage>().SetEnabled(true, charge_count_3);

        daenerys_left_cd_text = GetLinkedObject("daenerys_left_cd_text");
        ResetTextTimer(daenerys_left_cd_text);
    }

    public override void Update()
    {
        if (current_charges < max_charges)
        {
            //Perform Radial Fill when charges are 0
            if (current_charges == 0) 
            {
                //Manage the Radial Fill Cooldown
                float final_time = cd_time - act_time;
                if (final_time <= 0.0f)
                {
                    final_time = 0.0f;
                    reset_timer = true;
                }

                calc_time = final_time / cd_time;
                GetComponent<CompImage>().FillAmount(calc_time);

                //Manage Seconds Counter     
                ManageTextTimer(daenerys_left_cd_text);          

                //Reset Seconds Counter
                if (reset_timer)
                {
                    ResetTextTimer(daenerys_left_cd_text);
                    reset_timer = false;
                }
            }
           
            act_time += Time.deltaTime;
            
            if (act_time >= cd_time)
            {
                if (in_cd == true)
                {
                    in_cd = false;
                    button_cd = GetLinkedObject("daenerys_button_left_obj").GetComponent<CompButton>();
                    button_cd.Activate();

                    if (current_charges == 0)
                    {
                        GetComponent<CompImage>().FillAmount(1.0f);
                    }
                }

                current_charges++;

                UpdateChargesIcon();

                if (current_charges < max_charges)
                {
                    act_time = 0.0f;
                }
            }
        }
    }

    public override void OnClick()
    {
        if (GetLinkedObject("daenerys_obj").GetComponent<DaenerysController>().GetState() == 0
            && GetLinkedObject("player_obj").GetComponent<CharactersManager>().changing == false)
        {
            if (in_cd == false)
            {
                if (GetLinkedObject("daenerys_obj").GetComponent<DaenerysController>().OnLeftClick() == true)
                {
                    ActivateAbility();
                }                
            }
        }
    }

    public override void ActivateAbility()
    {
        current_charges--;
        UpdateChargesIcon();

        act_time = 0.0f;
        
        if (current_charges == 0)
        {
            button_cd = GetLinkedObject("daenerys_button_left_obj").GetComponent<CompButton>();
            button_cd.Deactivate();
            in_cd = true;
            prev_seconds = 1000;
        }
    }

    public int GetCurrentCharges()
    {
        return current_charges;
    }

    void UpdateChargesIcon()
    {
        switch(current_charges)
        {
            case 0:
                {
                    charge_count_0.GetComponent<CompImage>().SetEnabled(true, charge_count_0);
                    charge_count_1.GetComponent<CompImage>().SetEnabled(false, charge_count_1);
                    break;
                }
            case 1:
                {
                    charge_count_0.GetComponent<CompImage>().SetEnabled(false, charge_count_0);
                    charge_count_1.GetComponent<CompImage>().SetEnabled(true, charge_count_1);
                    charge_count_2.GetComponent<CompImage>().SetEnabled(false, charge_count_2);
                    break;
                }
            case 2:
                {
                    charge_count_1.GetComponent<CompImage>().SetEnabled(false, charge_count_1);
                    charge_count_2.GetComponent<CompImage>().SetEnabled(true, charge_count_2);
                    charge_count_3.GetComponent<CompImage>().SetEnabled(false, charge_count_3);
                    break;
                }
            case 3:
                {
                    charge_count_2.GetComponent<CompImage>().SetEnabled(false, charge_count_2);
                    charge_count_3.GetComponent<CompImage>().SetEnabled(true, charge_count_3);
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}