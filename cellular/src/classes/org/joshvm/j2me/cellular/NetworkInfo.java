/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
package org.joshvm.j2me.cellular;

public class NetworkInfo {
	private final int mnc;
	private final int mcc;
	private final int network_type;
	private final String operator_name;

	public static final int NETWORK_TYPE_3GPP = 81;
	public static final int NETWORK_TYPE_CDMA = 82;

	public NetworkInfo(int mnc, int mcc, int network_type, String operator_name) {
		this.mnc = mnc;
		this.mcc = mcc;
		this.network_type = network_type;
		this.operator_name = operator_name;
	}

	public int getMNC() {
		return mnc;
	}

	public int getMCC() {
		return mcc;
	}

	public int getNetworkType() {
		return network_type;
	}

	public String getOperatorName() {
		return operator_name;
	}
}

