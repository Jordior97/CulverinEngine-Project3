﻿using CulverinEditor;
using CulverinEditor.Debug;

public class DaenerysCD_Left : CoolDown
{
    public int max_charges = 3;

    private int current_charges = 3;

    public override void Update()
    {
        if (current_charges < max_charges)
        {
            act_time += Time.DeltaTime();
            if (act_time >= cd_time)
            {
                if (in_cd == true)
                {
                    in_cd = false;
                    button_cd = GetComponent<CompButton>();
                    button_cd.Activate();
                }
                current_charges++;
                act_time = 0.0f;
            }
        }
    }
    public override void OnClick()
    {
        if (GetLinkedObject("daenerys_obj").GetComponent<DaenerysController>().GetState() == 0)
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
        //this_obj.GetComponent
        
        Debug.Log("Daenerys Left CD Clicked");
        current_charges--;
        act_time = 0.0f;
        if (current_charges == 0)
        {
            button_cd = GetComponent<CompButton>();
            button_cd.Deactivate();
            in_cd = true;
        }
    }

    public int GetCurrentCharges()
    {
        return current_charges;
    }
}